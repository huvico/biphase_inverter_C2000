# Máquina de Estados – Controle de Motor F28379D

## Estados do sistema

```c
typedef enum
{
    INIT,
    RUNNING,
    STOPPING,
    ERROR,
    STOPPED,
    CALIBRATING,

} eSystemState;
```

### INIT

- Estado inicial após reset/power‑up.
- Ações principais:
  - `initialization()`
  - `Setup_GPIO()`
  - `Setup_PWM()`
  - `Setup_ADC()`
  - `Setup_eQEP()`
  - Ajuste de `w_nom`, `TS_RefGen` e inicialização do `rgen` (RefGen).
- Próximo estado: `STOPPED` (via `goto_STOPPED()`).

---

### STOPPED

- Descrição:
  - Sistema parado, conversor desligado.
  - PWM parado, contadores de ePWM congelados e duty cycle = 0.
  - Controle e interrupções de Timer0/ADC desabilitados.
- Ações típicas (em `goto_STOPPED()`):
  - `disable_control_interrupts()`
  - `stop_all_pwms()`
  - Zera variáveis principais de corrente/tensão (`adc1`, `adc2`, `V_alpha`, `V_beta`).
- É o estado seguro para aceitar comando de partida.

---

### RUNNING

- Descrição:
  - Sistema em operação normal.
  - PWM ativo, interrupções habilitadas.
  - RefGen gera `V_alpha` / `V_beta`, SVPWM calcula `wma`, `wmb`, `wmc`.
- Ações principais (em `end_init_goto_ON()`):
  - `start_all_pwms()`
  - `enable_control_interrupts()`
  - `DCL_resetRefgen(&rgen);`
  - `DCL_setRefgen(...);` com rampa de partida.
- No `RUNNING`, as referências são atualizadas na `isr_cpu_timer0()`:
  - `run_Refgen(&rgen, &V_alpha, &V_beta);`
  - `svpwm_bi(&teta, &V_alpha, &V_beta, &wma, &wmb, &wmc);`
  - Atualização de `EPwm1/2/3Regs.CMPA`.

---

### STOPPING

- Descrição:
  - Fase de parada controlada.
  - O RefGen é configurado para reduzir a amplitude da tensão de saída até zero, em rampa.
  - PWM ainda ativo, mas referência está diminuindo suavemente.
- Entrada em STOPPING:
  - `goto_OFF()` chamado a partir de `RUNNING` ao receber `Shutdown_command`.
  - `goto_OFF()` executa:
    ```c
    DCL_setRefgen(&rgen,
                  0.0f,
                  2.0f*M_PI*60.0f,
                  0.0f,
                  START_TIME_MACHINE/4.0f,
                  TS_RefGen);
    ```
- Condição de saída:
  - Quando `modulo < epsilon_modulo`, a máquina de estados chama `goto_STOPPED()`:
    ```c
    if (modulo < epsilon_modulo) {
        eNextState = goto_STOPPED();
    }
    ```
- Observação:
  - `turnOn_command` **não** é aceito em STOPPING; só é tratado em `STOPPED`.

---

### ERROR

- Descrição:
  - Estado de falha, principalmente disparado por sobrecorrente.
  - PWM desligado imediatamente e interrupções desabilitadas.
- Entrada em ERROR:
  - A partir de `RUNNING` quando `ReadEvent()` retorna `HI_Current`:
    ```c
    if (HI_Current == eNewEvent)
        eNextState = goto_error();
    ```
  - `goto_error()`:
    - `stop_all_pwms();`
    - `disable_control_interrupts();`
- Saída de ERROR:
  - Quando `ReadEvent()` retornar `turnOn_command`:
    ```c
    if (turnOn_command == eNewEvent)
        eNextState = end_init_goto_ON();
    ```
  - Ou seja, exige um comando explícito de partida para rearmar o sistema.

---

### CALIBRATING (planejado)

- Descrição:
  - Estado reservado para rotina de calibração (ex.: offsets de corrente).
  - A lógica está parcialmente comentada no código.
- Entrada prevista:
  - A partir de `STOPPED` quando `eNewEvent == turn_calibration`:
    ```c
    if (turn_calibration == eNewEvent)
        eNextState = goto_CALIBRATION();
    ```
- `goto_CALIBRATION()`:
  - `start_all_pwms();`
  - `enable_control_interrupts();`
  - `DCL_resetRefgen(&rgen);`
- Saída prevista (no trecho comentado do código):
  - Após realizar uma rotina de leitura repetida de ADC para calcular `offset1` e `offset2`.
  - Posível retorno via `goto_OFF()` → `STOPPING` → `STOPPED`.

---

## Eventos do sistema

Enum:

```c
typedef enum
{
    HI_Current,
    HI_DC_BUS_Voltage,
    Shutdown_command,
    turnOn_command,
    no_events,
    turn_calibration,

} eSystemEvent;
```

### Emitidos por `ReadEvent()` (estado global)

- **HI_Current**  
  - Condição:
    ```c
    if (adc1 > Imax || adc2 > Imax || adc1 < -Imax || adc2 < -Imax)
        return HI_Current;
    ```
  - Indica sobrecorrente nas medições de corrente.

- **Shutdown_command**  
  - Condição:
    ```c
    if (turn_off_command == 1) {
        DCL_setRefgen(...);   // rampa para zero
        turn_off_command = 0;
        return Shutdown_command;
    }
    ```
  - Command de parada, provavelmente vindo de interface externa.

- **turnOn_command**  
  - Condição:
    ```c
    if (turn_on_command == 1) {
        turn_on_command = 0;
        return turnOn_command;
    }
    ```
  - Comando de partida.

- **no_events**  
  - Padrão quando nenhuma condição acima é verdadeira.

- **turn_calibration**  
  - Trecho comentado; planejado para ativar estado de calibração.

- **HI_DC_BUS_Voltage**  
  - Ainda não emitido por `ReadEvent()`; reservado para implementação futura de proteção por sobretensão DC.

---

## Diagrama de estados (Mermaid)

Se sua ferramenta suporta Mermaid, você pode usar o diagrama abaixo diretamente:

```mermaid
stateDiagram-v2
    [*] --> INIT

    INIT --> STOPPED : após initialization + Setup_*

    STOPPED --> RUNNING : turnOn_command
    STOPPED --> CALIBRATING : turn_calibration

    RUNNING --> STOPPING : Shutdown_command
    RUNNING --> ERROR : HI_Current

    STOPPING --> STOPPED : modulo < epsilon_modulo

    ERROR --> RUNNING : turnOn_command

    state RUNNING {
        [*] --> RUNNING
    }

    state STOPPING {
        [*] --> STOPPING
    }

    state STOPPED {
        [*] --> STOPPED
    }

    state ERROR {
        [*] --> ERROR
    }

    state CALIBRATING {
        [*] --> CALIBRATING
    }
```

---

## Resumo operacional

- **Partida:**  
  1. Sistema nasce em `INIT`.  
  2. Após inicialização, vai para `STOPPED`.  
  3. Em `STOPPED`, recebendo `turnOn_command`, vai para `RUNNING`.

- **Parada normal em rampa:**  
  1. No estado `RUNNING`, recebendo `Shutdown_command`, chama `goto_OFF()` → `STOPPING`.  
  2. Em `STOPPING`, o RefGen reduz a amplitude até que `modulo < epsilon_modulo`.  
  3. A máquina de estados chama `goto_STOPPED()` → `STOPPED`, que desliga PWM e interrupções.

- **Parada por falha (sobreporcorrente):**  
  1. Em `RUNNING`, se `HI_Current`, vai para `ERROR`.  
  2. `ERROR` desliga PWM e interrupções imediatamente.  
  3. Para retomar, é necessário um novo `turnOn_command`, que leva a `RUNNING` via `end_init_goto_ON()`.
