# Trace-cmd Cheatsheet

## Índice
- [Trace-cmd Cheatsheet](#trace-cmd-cheatsheet)
  - [Índice](#índice)
  - [Introduccion](#introduccion)
  - [Comando Listar](#comando-listar)
    - [Listar los tracers disponibles](#listar-los-tracers-disponibles)
    - [Listar los eventos disponibles](#listar-los-eventos-disponibles)
      - [Listar formato de los eventos](#listar-formato-de-los-eventos)
      - [Listar los filtros disponibles](#listar-los-filtros-disponibles)
      - [Listar los triggers disponibles](#listar-los-triggers-disponibles)
    - [Listar las opciones de ftrace](#listar-las-opciones-de-ftrace)
    - [Listar los filtros de funciones disponibles](#listar-los-filtros-de-funciones-disponibles)
  - [Comando Status](#comando-status)
  - [Comando Start](#comando-start)
    - [Indicar tracer](#indicar-tracer)
    - [Indicar eventos de un subsistema](#indicar-eventos-de-un-subsistema)
    - [Indicar un evento concreto](#indicar-un-evento-concreto)
    - [Filtrado de funciones](#filtrado-de-funciones)
    - [Filtrado por PID](#filtrado-por-pid)
  - [Comando Stop](#comando-stop)
  - [Comando Show](#comando-show)
  - [Comando Reset](#comando-reset)
  - [Comando Record](#comando-record)
  - [Comando Report](#comando-report)
  - [Comando Extract](#comando-extract)

## Introduccion
Trace-cmd en una herramienta para shell que nos ayuda a configurar tracefs sin tener que montarlo ni escribir los distintos ficheros individualmente.

Es necesario instalarlo con `apt` si no estuviera instalado.

## Comando Listar
### Listar los tracers disponibles
Identico a [available-tracers](tracing.md#available_tracers)
```bash
user@mcn:~$ sudo trace-cmd list -t       
timerlat osnoise hwlat blk mmiotrace function_graph wakeup_dl wakeup_rt wakeup function nop
```
### Listar los eventos disponibles
Identico a [eventos-disponibles](events.md#eventos-disponibles)
```bash
user@mcn:~$ sudo trace-cmd list -e
```
Permite añadir expresiones regulares para filtrado
```bash
user@mcn:~$ trace-cmd list -e '^sig*'
```
#### Listar formato de los eventos
```bash
user@mcn:~$ sudo trace-cmd list -e '^sig*' -F
system: signal
name: signal_deliver
ID: 189
format:
	field:unsigned short common_type;	offset:0;	size:2;	signed:0;
	field:unsigned char common_flags;	offset:2;	size:1;	signed:0;
	field:unsigned char common_preempt_count;	offset:3;	size:1;	signed:0;
	field:int common_pid;	offset:4;	size:4;	signed:1;

	field:int sig;	offset:8;	size:4;	signed:1;
	field:int errno;	offset:12;	size:4;	signed:1;
	field:int code;	offset:16;	size:4;	signed:1;
	field:unsigned long sa_handler;	offset:24;	size:8;	signed:0;
	field:unsigned long sa_flags;	offset:32;	size:8;	signed:0;

system: signal
name: signal_generate
ID: 190
format:
	field:unsigned short common_type;	offset:0;	size:2;	signed:0;
	field:unsigned char common_flags;	offset:2;	size:1;	signed:0;
	field:unsigned char common_preempt_count;	offset:3;	size:1;	signed:0;
	field:int common_pid;	offset:4;	size:4;	signed:1;

	field:int sig;	offset:8;	size:4;	signed:1;
	field:int errno;	offset:12;	size:4;	signed:1;
	field:int code;	offset:16;	size:4;	signed:1;
	field:char comm[16];	offset:20;	size:16;	signed:0;
	field:pid_t pid;	offset:36;	size:4;	signed:1;
	field:int group;	offset:40;	size:4;	signed:1;
	field:int result;	offset:44;	size:4;	signed:1;
```

#### Listar los filtros disponibles
```bash
user@mcn:~$ sudo trace-cmd list -e '^sig*' -l
signal:signal_deliver
none

signal:signal_generate
none
```
#### Listar los triggers disponibles
```bash
user@mcn:~$ sudo trace-cmd list -e '^sig*' -R
signal:signal_deliver
# Available triggers:
# traceon traceoff snapshot stacktrace enable_event disable_event enable_hist disable_hist hist

signal:signal_generate
# Available triggers:
# traceon traceoff snapshot stacktrace enable_event disable_event enable_hist disable_hist hist
```

### Listar las opciones de ftrace
```bash
user@mcn:~$ sudo trace-cmd list -oamr@C16B:~$ sudo trace-cmd list -o
print-parent
nosym-offset
nosym-addr
noverbose
noraw
nohex
nobin
noblock
nofields
trace_printk
annotate
nouserstacktrace
nosym-userobj
noprintk-msg-only
context-info
nolatency-format
record-cmd
norecord-tgid
overwrite
nodisable_on_free
irq-info
markers
noevent-fork
nopause-on-trace
hash-ptr
function-trace
nofunction-fork
nodisplay-graph
nostacktrace
notest_nop_accept
notest_nop_refuse
```

### Listar los filtros de funciones disponibles
Identico a [filtro por funcion](tracing.md#filtro-por-funcion)

```bash
user@mcn:~$ sudo trace-cmd list -f
__traceiter_initcall_level
__probestub_initcall_level
__traceiter_initcall_start
__probestub_initcall_start
__traceiter_initcall_finish
__probestub_initcall_finish
trace_initcall_finish_cb
trace_initcall_start_cb
run_init_process
initcall_blacklisted
[...]
```

## Comando Status
Muesta un status general de todas las configuraciones de ftrace
```bash
user@mcn:~$ sudo trace-cmd stat

Tracer: function

Events:
 All disabled

Buffer size in kilobytes (per cpu):
   1410

Buffer total size in kilobytes:
   5640

Tracing is enabled

```

## Comando Start
Comienza un tracing ajustando las opciones del tracer

Formato
```bash
trace-cmd start -p <tracer> [-e <event>]
```

### Indicar tracer
```bash
user@mcn:~$ trace-cmd start -p function
```

### Indicar eventos de un subsistema
```bash
user@mcn:~$ trace-cmd start -e sched
```

### Indicar un evento concreto
```bash
user@mcn:~$ trace-cmd start -e sched_switch
```

### Filtrado de funciones
Permite definir que funciones se fitraran a la hora de capturar el tracer
```bash
user@mcn:~$ trace-cmd start -p function -l do_page_fault -l vfs_*
```

### Filtrado por PID
Identico a 
* set_ftrace_pid
* set_event_pid
```bash
user@mcn:~$ trace-cmd start -p function -P 900
```

```bash
user@mcn:~$ trace-cmd start -e all -P 900
```

## Comando Stop
Detiene el tracer
```bash
user@mcn:~$ trace-cmd stop
```

## Comando Show
Visualiza el contenido del buffer del tracer

```bash
user@mcn:~$ trace-cmd show
```

Opciones
* -P: usa trace_pipe
* -s: usa el fichero snapshot

## Comando Reset
Apaga todas las opciones de ftrace
* Dehabilita todos los tracers y eventos activos
* Limpia los buffers
* Elimina filtros y trigues configurados

```bash
user@mcn:~$ trace-cmd reset
```

## Comando Record
Acepta comando similares a `start` pero tiene mas funcionalidad.

Crea un proceso por CPU que almacena desde el buffer circular del kernel a un fichero por CPU.
```bash
user@mcn:~$ sudo ls /sys/kernel/tracing/per_cpu
cpu0  cpu1  cpu2  cpu3
```

Genera un fichero `trace.dat` que contiene parte del trace junto con informacion de tipo binario.

## Comando Report
Muestra por pantala el reporte creado mediante el comando `record`

## Comando Extract
Usado junto con los comandos `start` y `stop` para generar un fichero `trace.dat` desde el buffer circular de trace y poder mostrarlo mendiante el comando `report`