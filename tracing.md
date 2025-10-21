# Linux Tracing Cheatsheet (ftrace)
Ftrace permite rastrear y monitorizar las llamadas a funciones del kernel en tiempo real para debugging y análisis de rendimiento, mostrando el flujo de ejecución del código del kernel.

[mas info](https://www.kernel.org/doc/html/latest/trace/ftrace.html)

## Índice
- [Linux Tracing Cheatsheet (ftrace)](#linux-tracing-cheatsheet-ftrace)
  - [Índice](#índice)
  - [Configuraciones del Kernel necesarias](#configuraciones-del-kernel-necesarias)
  - [Ftrace en el sitema de ficheros](#ftrace-en-el-sitema-de-ficheros)
  - [Ficheros importantes en tracefs](#ficheros-importantes-en-tracefs)
    - [trace](#trace)
    - [trace\_pipe](#trace_pipe)
    - [available\_tracers](#available_tracers)
    - [current\_tracer](#current_tracer)
    - [buffer\_size\_kb](#buffer_size_kb)
  - [Cambiando el tracer activo](#cambiando-el-tracer-activo)
  - [Tipos de Tracer](#tipos-de-tracer)
    - [Tracer `function`](#tracer-function)
    - [Tracer `function_graph`](#tracer-function_graph)
      - [Duracion de la funcion](#duracion-de-la-funcion)
      - [Limitar la profundidad maxima de anidacion](#limitar-la-profundidad-maxima-de-anidacion)
  - [Borrado del buffer del tracer](#borrado-del-buffer-del-tracer)
  - [Encendiendo y apagando el tracing](#encendiendo-y-apagando-el-tracing)
  - [Acciones al ejecutar una funcion.](#acciones-al-ejecutar-una-funcion)
    - [agregar accion](#agregar-accion)
    - [borrar accion](#borrar-accion)
  - [Filtros](#filtros)
    - [Filtro por funcion](#filtro-por-funcion)
      - [Fijar un filtro](#fijar-un-filtro)
      - [Añadir un filtro](#añadir-un-filtro)
      - [Borrar todos los filtros](#borrar-todos-los-filtros)
    - [Filtrar las funciones de un modulo](#filtrar-las-funciones-de-un-modulo)
    - [Filtrar las funciones que llama una funcion del kernel](#filtrar-las-funciones-que-llama-una-funcion-del-kernel)
    - [Filtrar las funciones que llama un proceso](#filtrar-las-funciones-que-llama-un-proceso)
    - [Funciones que duren mas de un tiempo](#funciones-que-duren-mas-de-un-tiempo)
    - [Flags](#flags)
      - [irqs-off/BH-disabled (Posicion 1)](#irqs-offbh-disabled-posicion-1)
      - [need-resched (Posicion 2)](#need-resched-posicion-2)
      - [hardirq/softirq (Posicion 3)](#hardirqsoftirq-posicion-3)
      - [preempt-depth (Posicion 4)](#preempt-depth-posicion-4)
  - [Opciones de Trace](#opciones-de-trace)
    - [Habilitar opciones](#habilitar-opciones)
    - [Deshabilitar opciones](#deshabilitar-opciones)
    - [Directorio de opciones](#directorio-de-opciones)
    - [Estadisticas de uso de funciones](#estadisticas-de-uso-de-funciones)
  - [Trace desde espacio de usuario](#trace-desde-espacio-de-usuario)


## Configuraciones del Kernel necesarias
* `CONFIG_FTRACE` --> "Tracers"
* `CONFIG_FUNCTION_TRACER` --> Kernel Function Tracer
* `CONFIG_FUNCTION_GRAPH_TRACER` --> Kernel Function Graph Tracer
* `CONFIG_STACK_TRACER`	--> Traces Max stack

## Ftrace en el sitema de ficheros
Para Debian y derivados suele estar en `/sys/kernel/tracing/` o en kernel mas antiguos en `/sys/kernel/debug/tracing/`
```bash
user@mcn:~$ sudo ls -a /sys/kernel/tracing/
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
user@mcn:~$ sudo mount -t tracefs nodev /sys/kernel/tracing
```
o añadirlo a `/etc/fstab` para su montado automatico
```shell
 tracefs       /sys/kernel/tracing       tracefs defaults        0       0
```

## Ficheros importantes en tracefs
### trace
* Implementacion del buffer circular de ftrace en disco.
* Es un archivo estatico y los datos estaran disponibles despues de su lectura hasta que se sobreescriban.

### trace_pipe
* Al leerlo se consume el contenido por lo que solo se puede leer cada evento una vez.
* Es bloqueante: si no hay datos nuevos, espera a que lleguen.
* Util para monitorizacion en tiempo real

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
user@mcn:/sys/kernel/tracing$ cat available_tracers 
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

### buffer_size_kb
Tamaño del buffer en kb
```bash
user@mcn:~$ sudo cat /sys/kernel/tracing/buffer_size_kb
1410
```

## Cambiando el tracer activo
```bash
echo 'name of tracer' > current_tracer 
```
```bash
user@mcn:~$ sudo sh -c "echo 'function' > /sys/kernel/tracing/current_tracer"

user@mcn:~$ sudo cat /sys/kernel/tracing/current_tracer
function
```

## Tipos de Tracer

### Tracer `function`
```bash
user@mcn:~$ sudo head -20  /sys/kernel/tracing/trace
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

### Tracer `function_graph`
```bash
user@mcn:~$ sudo head -20 /sys/kernel/tracing/trace
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

#### Duracion de la funcion
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

Otra manera de buscar funciones que duren mas de un determinado tiempo es con [tracing_thresh](#funciones-que-duren-mas-de-un-tiempo) 

#### Limitar la profundidad maxima de anidacion
Mediante la configuracion de `/sys/kernel/tracing/max_grpht_depth` se puede limitar el maximo nivel de anidacion de las funciones.

Por defecto es 0

```bash
root@mcn:/sys/kernel/tracing# echo 2 > max_graph_depth 
root@mcn:/sys/kernel/tracing# echo function_graph > current_tracer 
root@mcn:/sys/kernel/tracing# head -50 trace

# tracer: function_graph
#
# CPU  DURATION                  FUNCTION CALLS
# |     |   |                     |   |   |   |
 3)   1.323 us    |  } /* irq_enter_rcu */
 3)               |  __sysvec_apic_timer_interrupt() {
 3) + 55.778 us   |    hrtimer_interrupt();
 3) + 58.500 us   |  }
 3)               |  irq_exit_rcu() {
 3) + 17.816 us   |    handle_softirqs();
 3)   1.273 us    |    sched_core_idle_cpu();
 3) + 22.622 us   |  }
 3)   1.278 us    |  syscall_exit_to_user_mode_prepare();
 3)   1.278 us    |  fpregs_assert_state_consistent();
 3)               |  syscall_trace_enter() {
 3)   2.691 us    |    __secure_computing();
 3)   5.303 us    |  }
 3)               |  x64_sys_call() {
 3) + 56.300 us   |    __x64_sys_write();
 3) + 58.836 us   |  }
 3)   1.227 us    |  syscall_exit_to_user_mode_prepare();
 3)   1.223 us    |  fpregs_assert_state_consistent();
 3)               |  syscall_trace_enter() {
 3)   7.838 us    |    __secure_computing();
 3)   9.973 us    |  }
 [...]
```

## Borrado del buffer del tracer
```bash
root@mcn:/sys/kernel/tracing# echo > /sys/kernel/tracing/trace
```

## Encendiendo y apagando el tracing
Es posible habilitarlo mendiante la escritura en el fichero `/sys/kernel/tracing/tracing_on`
* `# echo 0 > tracing_on`: apaga el tracing
* `# echo 1 > tracing_on`: activa el tracing

Ejemplo de uso durante test
```shell
 echo 0 > tracing_on
 echo function_graph > current_tracer
 echo 1 > tracing_on; run_test; echo 0 > tracing_on
```

## Acciones al ejecutar una funcion.
Es posible encender o apagar el tracing al inicio de una determinada funcion mediante la formuna `function:command[:count]`.
* function: es el nombre de la funcion que dispara el comando
* commad: Tipo de comando a ejecutar
  * traceon: activar el tracing
  * traceoff: desactivar el tracing
  * stacktrace: trazar el callstack de la funcion (similar a la opcion func_stack_trace)
* count: Opcional, indica el numero de veces que se repetira el comando.

### agregar accion
```bash
root@mcn:/sys/kernel/tracing# echo 'ksys_read:traceon:1' > set_ftrace_filter
```
### borrar accion
```bash
root@mcn:/sys/kernel/tracing# echo '!ksys_read:traceon:0' > set_ftrace_filter
```

## Filtros

### Filtro por funcion
todas las funciones del kernel que pueden ser filtradas estan exportadas en el fichero `/sys/kernel/tracing/available_filter_functions`
```bash
root@mcn:/sys/kernel/tracing# cat available_filter_functions | wc -l
78390
```
```bash
root@mcn:/sys/kernel/tracing# tail available_filter_functions
fill_sg_in.constprop.0 [tls]
complete_skb [tls]
chain_to_walk.isra.0 [tls]
tls_enc_record.constprop.0 [tls]
tls_enc_skb [tls]
tls_sw_fallback [tls]
tls_validate_xmit_skb [tls]
tls_encrypt_skb [tls]
tls_validate_xmit_skb_sw [tls]
tls_sw_fallback_init [tls]
```
Se indica tambien el modulo en el que se encuentra la funcion

* `set_ftrace_filter`: Solo se trazaran las funciones incluidas
* `set_ftrace_notrace`: No se trazaran las funciones incluidas

#### Fijar un filtro
```bash
root@mcn:/sys/kernel/tracing# echo 'function' > current_tracer 
root@mcn:/sys/kernel/tracing# echo 'kfree' > set_ftrace_filter 
```

```bash
root@mcn:/sys/kernel/tracing# head -20 trace
root@mcn:/sys/kernel/tracing# tracer: function
#
# entries-in-buffer/entries-written: 205069/218844   #P:4
#
#                                _-----=> irqs-off/BH-disabled
#                               / _----=> need-resched
#                              | / _---=> hardirq/softirq
#                              || / _--=> preempt-depth
#                              ||| / _-=> migrate-disable
#                              |||| /     delay
#           TASK-PID     CPU#  |||||  TIMESTAMP  FUNCTION
#              | |         |   |||||     |         |
            Xorg-1209    [002] .....  7118.665386: kfree <-drm_syncobj_array_wait_timeout.constprop.0
            Xorg-1209    [002] .....  7118.665387: kfree <-drm_syncobj_array_wait_timeout.constprop.0
            Xorg-1209    [002] .....  7118.665388: kfree <-drm_syncobj_array_free
            Xorg-1209    [002] .....  7118.665586: kfree <-drm_syncobj_array_find
            Xorg-1209    [002] .....  7118.665588: kfree <-drm_syncobj_array_wait_timeout.constprop.0
            Xorg-1209    [002] .....  7118.665588: kfree <-drm_syncobj_array_wait_timeout.constprop.0
            Xorg-1209    [002] .....  7118.665589: kfree <-drm_syncobj_array_free
            Xorg-1209    [002] .....  7118.665594: kfree <-drm_syncobj_array_find
```

#### Añadir un filtro
Tambien admite wildcards
```bash
root@mcn:/sys/kernel/tracing# echo vfs_* >> set_ftrace_filter
```

#### Borrar todos los filtros
```bash
root@mcn:/sys/kernel/tracing# echo > set_ftrace_filter 
```

### Filtrar las funciones de un modulo
```bash
root@mcn:/sys/kernel/tracing# echo ':mod:module_name' > set_ftrace_filter
```

### Filtrar las funciones que llama una funcion del kernel
Se puede obtener mediante el fichero `set_graph_function` y su opuesto `set_graph_notrace`

Funciona igual que `set_ftrace_filter` y `set_ftrace_notrace` pero para el tracer `function_graph`

```bash
root@mcn:/sys/kernel/tracing# echo 0 > tracing_on
root@mcn:/sys/kernel/tracing# echo function_graph > current_tracer
root@mcn:/sys/kernel/tracing# echo vfs_read > set_graph_function
root@mcn:/sys/kernel/tracing# echo 1 > tracing_on

root@mcn:/sys/kernel/tracing# cat trace
# tracer: function_graph
#
# CPU  DURATION                  FUNCTION CALLS
# |     |   |                     |   |   |   |
 0) + 10.795 us   |      } /* security_file_permission */
 0) + 15.115 us   |    } /* rw_verify_area */
 0)               |    pipe_read() {
 0)               |      mutex_lock() {
 0)   1.293 us    |        __cond_resched();
 0)   3.684 us    |      }
 0)   1.218 us    |      anon_pipe_buf_release();
 0)   1.213 us    |      mutex_unlock();
 0)   1.288 us    |      kill_fasync();
 0)               |      touch_atime() {
 0)               |        atime_needs_update() {
 0)   1.193 us    |          make_vfsuid();
 0)   1.213 us    |          make_vfsgid();
 0)               |          current_time() {
 0)   1.324 us    |            ktime_get_coarse_real_ts64();
 0)   3.684 us    |          }
 0) + 10.464 us   |        }
 0) + 12.815 us   |      }
 0) + 27.243 us   |    }
 0) + 46.637 us   |  } /* vfs_read */
 0)               |  vfs_read() {
 0)               |    rw_verify_area() {
 0)               |      security_file_permission() {
 0)               |        apparmor_file_permission() {
 0)               |          aa_file_perm() {
 0)   0.772 us    |            __rcu_read_lock();
 0)   0.677 us    |            __rcu_read_unlock();
 0)   3.799 us    |          }
 0)   5.367 us    |        }
 0)   6.846 us    |      }
 0)   9.156 us    |    }
 0)               |    pipe_read() {
 0)               |      mutex_lock() {
 0)   0.726 us    |        __cond_resched();
 0)   2.085 us    |      }
 0)   1.018 us    |      anon_pipe_buf_release();
 0)   0.747 us    |      mutex_unlock();
 0)   0.702 us    |      kill_fasync();
 0)               |      touch_atime() {
 0)               |        atime_needs_update() {
 0)   0.707 us    |          make_vfsuid();
 0)   0.722 us    |          make_vfsgid();
 0)               |          current_time() {
 0)   0.737 us    |            ktime_get_coarse_real_ts64();
 0)   2.080 us    |          }
 0)   6.145 us    |        }
 0)   0.717 us    |        mnt_get_write_access();
 0)               |        generic_update_time() {
 0)               |          inode_update_timestamps() {
 0)   0.702 us    |            ktime_get_coarse_real_ts64();
 0)   0.747 us    |            timestamp_truncate();
 0)   3.398 us    |          }
 0)   0.731 us    |          __mark_inode_dirty();
 0)   5.953 us    |        }
 0)   0.787 us    |        mnt_put_write_access();
 0) + 17.069 us   |      }
 0) + 26.025 us   |    }
 0) + 39.717 us   |  }
 [...]

```

### Filtrar las funciones que llama un proceso
Mediante la configuracion del fichero `set_ftrace_pid`

Ejemplo ver las llamadas que se ejecutan por una llamada desde el shell mediante echo $$ ($$ indica el pid del shell actual)

```bash
#!/bin/bash

DEBUGFS=`grep debugfs /proc/mounts | awk '{print $2;}'`
echo nop > $DEBUGFS/tracing/current_tracer
echo > $DEBUGFS/tracing/trace
echo 0 > $DEBUGFS/tracing/tracing_on
echo $1
echo $$ > $DEBUGFS/tracing/set_ftrace_pid
echo function > $DEBUGFS/tracing/current_tracer
echo 1 > $DEBUGFS/tracing/tracing_on
exec $*
echo 0 > $DEBUGFS/tracing/tracing_on
```

```bash
user@mcn:~$ sudo ./traceme.sh ls
ls
notes.txt  traceme.sh

user@mcn:~$ sudo cat /sys/kernel/tracing/trace
# tracer: function
#
# entries-in-buffer/entries-written: 36611/36611   #P:4
#
#                                _-----=> irqs-off/BH-disabled
#                               / _----=> need-resched
#                              | / _---=> hardirq/softirq
#                              || / _--=> preempt-depth
#                              ||| / _-=> migrate-disable
#                              |||| /     delay
#           TASK-PID     CPU#  |||||  TIMESTAMP  FUNCTION
#              | |         |   |||||     |         |
              ls-63594   [003] ..... 13532.067988: mutex_unlock <-rb_simple_write
              ls-63594   [003] ..... 13532.067990: syscall_exit_to_user_mode_prepare <-syscall_exit_to_user_mode
              ls-63594   [003] ..... 13532.067990: mem_cgroup_handle_over_high <-syscall_exit_to_user_mode
              ls-63594   [003] ..... 13532.067990: blkcg_maybe_throttle_current <-syscall_exit_to_user_mode
              ls-63594   [003] ..... 13532.067990: __rseq_handle_notify_resume <-syscall_exit_to_user_mode
              ls-63594   [003] ..... 13532.067991: rseq_ip_fixup <-__rseq_handle_notify_resume
              ls-63594   [003] ..... 13532.067991: rseq_get_rseq_cs <-rseq_ip_fixup
              ls-63594   [003] ..... 13532.067991: rseq_update_cpu_node_id <-__rseq_handle_notify_resume
              ls-63594   [003] d.... 13532.067992: fpregs_assert_state_consistent <-syscall_exit_to_user_mode
              ls-63594   [003] d.... 13532.067992: switch_fpu_return <-syscall_exit_to_user_mode
              ls-63594   [003] ..... 13532.067998: x64_sys_call <-do_syscall_64
              ls-63594   [003] ..... 13532.067999: __x64_sys_dup2 <-x64_sys_call
              ls-63594   [003] ..... 13532.067999: ksys_dup3 <-__x64_sys_dup2
              ls-63594   [003] ..... 13532.067999: _raw_spin_lock <-ksys_dup3
              ls-63594   [003] ...1. 13532.068000: expand_files <-ksys_dup3
              ls-63594   [003] ...1. 13532.068000: do_dup2 <-ksys_dup3
              ls-63594   [003] ...1. 13532.068001: _raw_spin_unlock <-do_dup2
[...]
```

```bash
user@mcn:~$ sudo cat /sys/kernel/tracing/trace | wc -l
36623
```

### Funciones que duren mas de un tiempo
```bash
root@mcn:/sys/kernel/tracing# echo 100000 > tracing_thresh 
root@mcn:/sys/kernel/tracing# echo function_graph > current_tracer 
root@mcn:/sys/kernel/tracing# head -60 trace

# tracer: function_graph
#
# CPU  DURATION                  FUNCTION CALLS
# |     |   |                     |   |   |   |
 2) @ 100140.3 us |  } /* x64_sys_call */
 2) @ 100139.0 us |        } /* do_poll.constprop.0 */
 2) @ 100143.6 us |      } /* do_sys_poll */
 2) @ 100145.5 us |    } /* __x64_sys_poll */
 2) @ 100146.4 us |  } /* x64_sys_call */
 ------------------------------------------
 2)  Inotify-3838  =>   code-7840   
 ------------------------------------------

 2) @ 200211.4 us |  } /* x64_sys_call */
 ------------------------------------------
 2)   code-7840    =>   code-7822   
 ------------------------------------------

 2) @ 200297.9 us |            } /* futex_wait_queue */
 2) @ 200300.2 us |          } /* __futex_wait */
 2) @ 200303.2 us |        } /* futex_wait */
 2) @ 200303.8 us |      } /* do_futex */
 2) @ 200304.7 us |    } /* __x64_sys_futex */
 2) @ 200305.1 us |  } /* x64_sys_call */
[...]
```

### Flags
Los flags proporcionan informacion sobre el estado del sistema en el momento de la llamada a funcion.

```bash
# tracer: function
#
# entries-in-buffer/entries-written: 205028/14641083   #P:4
#
#                                _-----=> irqs-off/BH-disabled
#                               / _----=> need-resched
#                              | / _---=> hardirq/softirq
#                              || / _--=> preempt-depth
#                              ||| / _-=> migrate-disable
#                              |||| /     delay
#           TASK-PID     CPU#  |||||  TIMESTAMP  FUNCTION
#              | |         |   |||||     |         |
   IPC I/O Child-7360    [003] d.s1. 10081.902716: sugov_iowait_boost <-sugov_update_single_freq
   IPC I/O Child-7360    [003] d.s1. 10081.902716: sugov_should_update_freq <-sugov_update_single_freq
   IPC I/O Child-7360    [003] d.s1. 10081.902717: sugov_iowait_apply.constprop.0 <-sugov_update_single_freq
   IPC I/O Child-7360    [003] d.s1. 10081.902717: sugov_get_util <-sugov_update_single_freq
   IPC I/O Child-7360    [003] d.s1. 10081.902717: cpu_util_cfs_boost <-sugov_get_util
   IPC I/O Child-7360    [003] d.s1. 10081.902718: effective_cpu_util <-sugov_get_util
   IPC I/O Child-7360    [003] d.s1. 10081.902719: get_next_freq.constprop.0 <-sugov_update_single_freq
   IPC I/O Child-7360    [003] d.s1. 10081.902719: cpufreq_driver_resolve_freq <-get_next_freq.constprop.0
   IPC I/O Child-7360    [003] d.s1. 10081.902719: tick_nohz_get_idle_calls_cpu <-sugov_update_single_freq
   IPC I/O Child-7360    [003] d.s1. 10081.902720: raw_spin_rq_unlock <-update_blocked_averages
   IPC I/O Child-7360    [003] d.s1. 10081.902720: _raw_spin_unlock <-raw_spin_rq_unlock
   IPC I/O Child-7360    [003] ..s.. 10081.902721: rebalance_domains <-run_rebalance_domains
   IPC I/O Child-7360    [003] ..s.. 10081.902721: __rcu_read_lock <-rebalance_domains
   IPC I/O Child-7360    [003] ..s.. 10081.902722: __msecs_to_jiffies <-rebalance_domains
   IPC I/O Child-7360    [003] ..s.. 10081.902722: __msecs_to_jiffies <-rebalance_domains
   IPC I/O Child-7360    [003] ..s.. 10081.902723: __rcu_read_unlock <-rebalance_domains
   IPC I/O Child-7360    [003] d.... 10081.902723: sched_core_idle_cpu <-__irq_exit_rcu
   IPC I/O Child-7360    [003] ..... 10081.902724: unix_destruct_scm <-skb_release_head_state
   IPC I/O Child-7360    [003] ..... 10081.902725: put_pid <-unix_destruct_scm
   IPC I/O Child-7360    [003] ..... 10081.902725: sock_wfree <-unix_destruct_scm

```

#### irqs-off/BH-disabled (Posicion 1)
Indica si las interrupciones están deshabilitadas o si los bottom halves están deshabilitados:
* `.`: Interrupciones habilitadas y bottom halves habilitados
* `d`: Interrupciones deshabilitadas (IRQs off)

#### need-resched (Posicion 2)  
Indica si el proceso actual necesita ser replanificado:
* `N`: TIF_NEED_RESCHED y PREEMPT_NEED_RESCHED están establecidos
* `n`: solo TIF_NEED_RESCHED está establecido  
* `p`: solo PREEMPT_NEED_RESCHED está establecido
* `.`: ninguno está establecido

* `TIF_NEED_RESCHED`: indica que el proceso lleva suficiente tiempo de ejecucion y necesita replanificarse pero continua ejecutandose. No se interrumpe inmediatamente.
* `PREEMPT_NEED_RESCHED`: indica que el proceso actual debe ser reemplazado por otra de mas prioridad.

#### hardirq/softirq (Posicion 3)
Muestra el contexto de interrupcion actual:
* `.`: Contexto normal (process context)
* `s`: Ejecutandose en contexto de soft interrupt
* `h`: Ejecutandose en contexto de hard interrupt
* `H`: Hard interrupt ocurrio durante una soft interrupt
* `z`: NMI esta ejecutandose
* `Z`: NMI sucedio durante una hard interrupt

#### preempt-depth (Posicion 4)
Indica el nivel de deshabilitación del preemption:
* `.` = preemption habilitada (depth = 0)
* `0-f` = Nivel de preempt_count en hexadecimal (cuántas veces se deshabilito el preemption)
* `#` = Nivel mayor a 15

Estos flags son importantes para entender el contexto de ejecucion y detectar problemas como:
- Secciones criticas muy largas (IRQs deshabilitadas por mucho tiempo)
- Problemas de latencia (need-resched no atendido)
- Problemas de concurrencia (preemption deshabilitado)

## Opciones de Trace
Es posible modificar las opciones en la que se registran y muestran los trace mediante el fichero `trace_options`.

Cada tipo de tracing tiene distinto numero de opciones.

```bash
root@mcn:/sys/kernel/tracing#cat current_tracer 
nop

root@mcn:/sys/kernel/tracing#cat trace_options 
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
Las opciones que empiezan por `no` estan desabilitadas. Pej `nostacktrace`

### Habilitar opciones
```bash
root@mcn:/sys/kernel/tracing# echo irq-info > trace_options
```

### Deshabilitar opciones
```bash
root@mcn:/sys/kernel/tracing# echo noirq-info > trace_options
```

### Directorio de opciones
Tambien es posible habilitar o deshabilitar opciones escribiendo `1`o `0` en los ficheros de opciones dentro del directorio `options`

```bash
root@mcn:/sys/kernel/tracing# echo 1 > options/irq-info
```
```bash
root@mcn:/sys/kernel/tracing# echo 0 > options/irq-info 
```

### Estadisticas de uso de funciones
Tiene que estar `function` seleccionado como `current_profiler` Se habilita en el fichero `function_profile_enabled`

Las estadisticas se guardan en el directorio `/sys/kernel/tracing/trace_stat`en un fichero por cada CPU del procesador

```bash
root@mcn:/sys/kernel/tracing# echo function > current_tracer 
root@mcn:/sys/kernel/tracing# echo 1 > function_profile_enabled 
root@mcn:/sys/kernel/tracing# ls ./trace_stat/
function0  function1  function2  function3

root@mcn:/sys/kernel/tracing# head -10 ./trace_stat/function0
  Function                               Hit    Time            Avg             s^2
  --------                               ---    ----            ---             ---
  schedule                             28034    1970681373 us     70296.11 us     7554.095 us 
  x64_sys_call                        122777    1656612969 us     13492.86 us     473.008 us  
  schedule_hrtimeout_range             10463    1063109603 us     101606.5 us     158132.0 us 
  schedule_hrtimeout_range_clock       10463    1063095878 us     101605.2 us     35196.36 us 
  do_sys_poll                          17498    585450538 us     33458.14 us     23609.03 us 
  do_poll.constprop.0                  17498    585251716 us     33446.77 us     39894.13 us 
  __x64_sys_poll                       17407    566162986 us     32525.01 us     32502.54 us 
  __x64_sys_futex                      22145    565463403 us     25534.58 us     10152.44 us 
```

## Trace desde espacio de usuario
Es posible hacerlo escribiendo en `trace_marker`

Desde el shell
```bash
root@mcn:/sys/kernel/tracing# echo nop > current_tracer 

root@mcn:/sys/kernel/tracing# echo "hello world" > trace_marker
cat: tracer: No existe el archivo o el directorio

root@mcn:/sys/kernel/tracing# cat trace
# tracer: nop
#
# entries-in-buffer/entries-written: 1/1   #P:4
#
#                                _-----=> irqs-off/BH-disabled
#                               / _----=> need-resched
#                              | / _---=> hardirq/softirq
#                              || / _--=> preempt-depth
#                              ||| / _-=> migrate-disable
#                              |||| /     delay
#           TASK-PID     CPU#  |||||  TIMESTAMP  FUNCTION
#              | |         |   |||||     |         |
           <...>-9066    [003] .....  7666.092874: tracing_mark_write: hello world
```

Desde programas de usuario.
```c
static int marker_fd = -1;
[...]
{
    marker_fd = open("/sys/kernel/tracing/trace_marker", O_WRONLY);
    if (marker_fd < 0) {
        perror("error:");
        exit(-1);
	  }
    write(marker_fd, buf, buff_size);
    [...]
}
```