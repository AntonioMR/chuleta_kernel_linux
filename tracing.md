# Linux Tracing Cheatsheet
Ftrace permite rastrear y monitorizar las llamadas a funciones del kernel en tiempo real para debugging y análisis de rendimiento, mostrando el flujo de ejecución del código del kernel.

[mas info](https://www.kernel.org/doc/html/latest/trace/ftrace.html)

## Índice
- [Linux Tracing Cheatsheet](#linux-tracing-cheatsheet)
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
  - [Borrado del buffer del tracer](#borrado-del-buffer-del-tracer)
  - [Encendiendo y apagando el tracing](#encendiendo-y-apagando-el-tracing)
  - [Filtros](#filtros)
    - [Filtro por funcion](#filtro-por-funcion)
      - [Fijar un filtro](#fijar-un-filtro)
      - [Añadir un filtro](#añadir-un-filtro)
      - [Borrar todos los filtros](#borrar-todos-los-filtros)
    - [Filtrar las funciones que llama una funcion del kernel](#filtrar-las-funciones-que-llama-una-funcion-del-kernel)
    - [Filtrar las funciones que llama un proceso](#filtrar-las-funciones-que-llama-un-proceso)


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

### buffer_size_kb
Tamaño del buffer en kb
```bash
 sudo cat /sys/kernel/tracing/buffer_size_kb
1410
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

## Tipos de Tracer

### Tracer `function`
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

### Tracer `function_graph`
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

## Borrado del buffer del tracer
```bash
# echo > /sys/kernel/tracing/trace
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

## Filtros

### Filtro por funcion
todas las funciones del kernel que pueden ser filtradas estan exportadas en el fichero `/sys/kernel/tracing/available_filter_functions`
```bash
# cat available_filter_functions | wc -l
78390
```
```bash
# tail available_filter_functions
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
# echo 'function' > current_tracer 
# echo 'kfree' > set_ftrace_filter 
```

```bash
# head -20 trace
# tracer: function
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
# echo vfs_* >> set_ftrace_filter
```

#### Borrar todos los filtros
```bash
# echo > set_ftrace_filter 
```

### Filtrar las funciones que llama una funcion del kernel
Se puede obtener mediante el fichero `set_graph_function` y su opuesto `set_graph_notrace`

Funciona igual que `set_ftrace_filter` y `set_ftrace_notrace` pero para el tracer `function_graph`

```bash
# echo 0 > tracing_on
# echo function_graph > current_tracer
# echo vfs_read > set_graph_function
# echo 1 > tracing_on

# cat trace
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
$ sudo ./traceme.sh ls
ls
notes.txt  traceme.sh

$ sudo cat /sys/kernel/tracing/trace
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
$ sudo cat /sys/kernel/tracing/trace | wc -l
36623
```
