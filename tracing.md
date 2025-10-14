# Linux Tracing Cheatsheet
Ftrace permite rastrear y monitorizar las llamadas a funciones del kernel en tiempo real para debugging y análisis de rendimiento, mostrando el flujo de ejecución del código del kernel.

## Índice
- [Linux Tracing Cheatsheet](#linux-tracing-cheatsheet)
  - [Índice](#índice)
  - [Configuraciones del Kernel necesarias](#configuraciones-del-kernel-necesarias)
  - [Ftrace en el sitema de ficheros](#ftrace-en-el-sitema-de-ficheros)
  - [Ficheros importantes en tracefs](#ficheros-importantes-en-tracefs)
    - [trace](#trace)
    - [available\_tracers](#available_tracers)
    - [current\_tracer](#current_tracer)
  - [Cambiando el tracer activo](#cambiando-el-tracer-activo)
  - [Tracer `function`](#tracer-function)
  - [Tracer `function_graph`](#tracer-function_graph)
    - [Duracion de la funcion](#duracion-de-la-funcion)


## Configuraciones del Kernel necesarias
`CONFIG_FTRACE` --> "Tracers"
`CONFIG_FUNCTION_TRACER` --> Kernel Function Tracer
`CONFIG_FUNCTION_GRAPH_TRACER` --> Kernel Function Graph Tracer
`CONFIG_STACK_TRACER`	--> Traces Max stack

## Ftrace en el sitema de ficheros
Para Debian y derivados suele estar en `/sys/kernel/tracing/` o en kernel mas antiguos en `/sys/kernel/debug/tracing/`
```bash
$ sudo ls -a /sys/kernel/tracing/
.                                   enabled_functions           printk_formats          set_graph_function	trace_options
..                                  error_log                   README                  set_graph_notrace	trace_pipe
available_events                    events                      rv                      snapshot		    trace_stat
available_filter_functions          free_buffer                 saved_cmdlines          stack_max_size	    tracing_cpumask
available_filter_functions_addrs    function_profile_enabled    saved_cmdlines_size     stack_trace		    tracing_max_latency
available_tracers                   hwlat_detector              saved_tgids             stack_trace_filter  tracing_on
buffer_percent                      instances                   set_event               synthetic_events	tracing_thresh
buffer_size_kb                      kprobe_events               set_event_notrace_pid   timestamp_mode	    uprobe_events
buffer_subbuf_size_kb               kprobe_profile              set_event_pid           touched_functions	uprobe_profile
buffer_total_size_kb                max_graph_depth             set_ftrace_filter	    trace		        user_events_data
current_tracer                      options                     set_ftrace_notrace	    trace_clock		    user_events_status
dynamic_events                      osnoise                     set_ftrace_notrace_pid  trace_marker
dyn_ftrace_total_info               per_cpu                     set_ftrace_pid          trace_marker_raw
```
Si no estuviera montado habria que montarlo manualmente
```bash
$ sudo mount -t tracefs nodev /sys/kernel/tracing
```
o añadirlo a `/etc/fstab` para su montado automatico
```shell
 tracefs       /sys/kernel/tracing       tracefs defaults        0       0
```

## Ficheros importantes en tracefs
### trace
Implementacion del buffer circular de ftrace en disco.

```bash
cat trace
# tracer: nop
#
# entries-in-buffer/entries-written: 0/0   #P:4
#
#                                _-----=> irqs-off/BH-disabled
#                               / _----=> need-resched
#                              | / _---=> hardirq/softirq
#                              || / _--=> preempt-depth
#                              ||| / _-=> migrate-disable
#                              |||| /     delay
#           TASK-PID     CPU#  |||||  TIMESTAMP  FUNCTION
#              | |         |   |||||     |         |
```

### available_tracers
lista de tracert configurados y disponibles
```bash
$ cat available_tracers 
timerlat osnoise hwlat blk mmiotrace function_graph wakeup_dl wakeup_rt wakeup function nop
```
Cada tracer se enfoca en aspectos específicos del rendimiento y comportamiento del kernel:
* nop - Tracer "no operation", desactiva el trazado. Es el tracer por defecto cuando no se quiere rastrear nada.
* function - Rastrea todas las llamadas a funciones del kernel, mostrando qué funciones se ejecutan.
* function_graph - Versión avanzada de function que muestra las llamadas de funciones en formato gráfico con entrada/salida y tiempos de ejecución.
* wakeup - Rastrea la latencia máxima desde que un proceso se despierta hasta que realmente se ejecuta.
* wakeup_rt - Similar a wakeup pero específico para procesos de tiempo real (RT).
* wakeup_dl - Similar a wakeup pero específico para procesos con deadline scheduling.
* mmiotrace - Rastrea accesos a memoria mapeada de dispositivos (MMIO), útil para debugging de drivers.
* blk - Rastrea operaciones del subsistema de bloques (discos, I/O).
* hwlat - Detecta latencias de hardware midiendo interrupciones en bucles de polling.
* osnoise - Mide el "ruido" del sistema operativo que puede afectar aplicaciones de tiempo real.
* timerlat - Mide la latencia de los timers del kernel, útil para análisis de tiempo real.

### current_tracer
Tracer en uso en este momento.
```bash
cat current_tracer 
nop
```

## Cambiando el tracer activo
```bash
echo 'name of tracer' > current_tracer 
```
```bash
$ sudo sh -c "echo 'function' > /sys/kernel/tracing/current_tracer"

$ sudo cat /sys/kernel/tracing/current_tracer
function
```

## Tracer `function`
```bash
$ sudo head -20  /sys/kernel/tracing/trace
# tracer: function
#
# entries-in-buffer/entries-written: 205028/367225258   #P:4
#
#                                _-----=> irqs-off/BH-disabled
#                               / _----=> need-resched
#                              | / _---=> hardirq/softirq
#                              || / _--=> preempt-depth
#                              ||| / _-=> migrate-disable
#                              |||| /     delay
#           TASK-PID     CPU#  |||||  TIMESTAMP  FUNCTION
#              | |         |   |||||     |         |
            gala-2680    [003] ..... 11288.353412: ____sys_recvmsg <-___sys_recvmsg
            gala-2680    [003] ..... 11288.353413: sock_recvmsg <-____sys_recvmsg
            gala-2680    [003] ..... 11288.353413: security_socket_recvmsg <-sock_recvmsg
            gala-2680    [003] ..... 11288.353413: apparmor_socket_recvmsg <-security_socket_recvmsg
            gala-2680    [003] ..... 11288.353413: aa_unix_msg_perm <-apparmor_socket_recvmsg
            gala-2680    [003] ..... 11288.353413: unix_stream_recvmsg <-sock_recvmsg
            gala-2680    [003] ..... 11288.353413: unix_stream_read_generic <-unix_stream_recvmsg
            gala-2680    [003] ..... 11288.353413: mutex_lock <-unix_stream_read_generic
```
En el trace se puede ver:
* Nombre y pid del proceso: Nombre (gala) PID (2680)
* CPU en la que se esta ejecutando: CPU 3
* Flags: Ningun flag activo
* Timestamp desde el inicio del sistema en el que se llama a la funcion: 11288.353412
* Llamada de funcion: Funcion ___sys_recvmsg(...) llama a la funcion ____sys_recvmsg(...)

## Tracer `function_graph`
```bash
$ sudo head -20 /sys/kernel/tracing/trace
# tracer: function_graph
#
# CPU  DURATION                  FUNCTION CALLS
# |     |   |                     |   |   |   |
 3)   1.021 us    |                  } /* psi_group_change */
 3)               |                  psi_group_change() {
 3)   0.368 us    |                    record_times();
 3)   1.123 us    |                  }
 3)               |                  psi_group_change() {
 3)   0.342 us    |                    record_times();
 3)   1.111 us    |                  }
 3)               |                  psi_group_change() {
 3)   0.363 us    |                    record_times();
 3)   1.179 us    |                  }
 3)   9.925 us    |                } /* psi_task_switch */
 3)   0.558 us    |                __traceiter_sched_switch();
 3)   0.348 us    |                enter_lazy_tlb();
 3)               |                save_fpregs_to_fpstate() {
 3)   0.377 us    |                  xfd_validate_state();
 3)   1.159 us    |                }
```
En el trace se puede ver:
* CPU en la que se esta ejecutando: CPU 3
* Duracion de la ejecucion de la funcion
* Entrada a la funcion con llamada a otras funciones `{`. El tiempo de ejecucion se muestra a la salida
* Ejecucion de funciones que no llaman a otras funciones `;` (se indica el tiempo de ejecucion)
* Salida de funcion que llama a otras funciones `}`. Se muestra el tiempo total de la funcion

La funcion `save_fpregs_to_fpstate()` tiene un tiempo de ejecucion de 1.159 us y hace una llamada a `xfd_validate_state()` con un tiempo de ejecucion de 0.377 us

### Duracion de la funcion
* Cuando la duración es mayor de 10 microsegundos, se muestra un `+` en la columna DURATION  
* Cuando la duración es mayor de 100 microsegundos, se muestra un `-` en la columna DURATION  
* Cuando la duración es mayor de 1000 microsegundos, se muestra un `#` en la columna DURATION  
* Cuando la duración es mayor de 10 milisegundos, se muestra un `*` en la columna DURATION  
* Cuando la duración es mayor de 100 milisegundos, se muestra un `@` en la columna DURATION  
* Cuando la duración es mayor de 1 segundo, se muestra un `$` en la columna DURATION  
  
Buscar funciones que duren mas de 10 milisegundos

```bash
sudo cat /sys/kernel/tracing/trace | grep ' \* '
 0) * 10511.78 us |              }
 0) * 10517.91 us |            } /* futex_wait_queue */
 0) * 10526.15 us |          } /* __futex_wait */
 0) * 10531.61 us |        } /* futex_wait */
 0) * 10532.34 us |      } /* do_futex */
 0) * 10533.88 us |    } /* __x64_sys_futex */
 0) * 10534.61 us |  }
```