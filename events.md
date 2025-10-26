# Linux Tracepoints Cheatsheet
Los Trace Events en Linux son un mecanismo mas avanzado que ftracing que permiten mostrar parametros y variables locales. Trace Events estan basados en tracepoints que nos unos ganchos insertados en el codigo que pueden ser activados externamente y tienen un bajo impacto cuando estan desactivados

[mas info](https://www.kernel.org/doc/html/latest/trace/events.html)

## Indice
- [Linux Tracepoints Cheatsheet](#linux-tracepoints-cheatsheet)
  - [Indice](#indice)
  - [TRACE\_EVENT macro](#trace_event-macro)
  - [Eventos disponibles](#eventos-disponibles)
  - [Habilitar eventos](#habilitar-eventos)
  - [Deshabilitar eventos](#deshabilitar-eventos)
  - [Habilitar/Deshabilitar todos los eventos de un subsistema](#habilitardeshabilitar-todos-los-eventos-de-un-subsistema)
  - [Consultar estado del evento](#consultar-estado-del-evento)
  - [Contenido de un evento](#contenido-de-un-evento)
    - [enable](#enable)
    - [id](#id)
    - [format](#format)
    - [filter](#filter)
      - [Añadir un filtro](#añadir-un-filtro)
      - [Borrar los filtos](#borrar-los-filtos)
      - [Operadores validos](#operadores-validos)
    - [Trigger](#trigger)
      - [Añadir un trigger](#añadir-un-trigger)
      - [Eliminar un trigger](#eliminar-un-trigger)
      - [Comandos del trigger](#comandos-del-trigger)
        - [Enable\_event](#enable_event)
        - [Disable\_event](#disable_event)
        - [stacktrace](#stacktrace)
        - [hist](#hist)
        - [Comandos disponibles](#comandos-disponibles)
    - [hist](#hist-1)
    - [inject](#inject)
  - [Habilitar eventos con `set_event`](#habilitar-eventos-con-set_event)

## TRACE_EVENT macro
El kernel utiliza la macro `TRACE_EVENT` para definir las esctruturas del tracepoint que sera incluido en el codigo. Tambien crea la funcion callback que sera llamada para traducir e insertar los datos en el buffer circular del tracert

```c
TRACE_EVENT(sched_switch,

        TP_PROTO(bool preempt,
                 struct task_struct *prev,
                 struct task_struct *next,
                 unsigned int prev_state),

        TP_ARGS(preempt, prev, next, prev_state),

        TP_STRUCT__entry(
                __array(        char,   prev_comm,      TASK_COMM_LEN   )
                __field(        pid_t,  prev_pid                        )
                __field(        int,    prev_prio                       )
                __field(        long,   prev_state                      )
                __array(        char,   next_comm,      TASK_COMM_LEN   )
                __field(        pid_t,  next_pid                        )
                __field(        int,    next_prio                       )
        ),

        TP_fast_assign(
                memcpy(__entry->next_comm, next->comm, TASK_COMM_LEN);
                __entry->prev_pid       = prev->pid;
                __entry->prev_prio      = prev->prio;
                __entry->prev_state     = __trace_sched_switch_state(preempt, prev_state, prev);
                memcpy(__entry->prev_comm, prev->comm, TASK_COMM_LEN);
                __entry->next_pid       = next->pid;
                __entry->next_prio      = next->prio;
                /* XXX SCHED_DEADLINE */
        ),
        
        TP_printk("prev_comm=%s prev_pid=%d prev_prio=%d prev_state=%s%s ==> next_comm=%s next_pid=%d next_prio=%d",
                __entry->prev_comm, __entry->prev_pid, __entry->prev_prio,

                (__entry->prev_state & (TASK_REPORT_MAX - 1)) ?
                  __print_flags(__entry->prev_state & (TASK_REPORT_MAX - 1), "|",
                                { TASK_INTERRUPTIBLE, "S" },
                                { TASK_UNINTERRUPTIBLE, "D" },
                                { __TASK_STOPPED, "T" },
                                { __TASK_TRACED, "t" },
                                { EXIT_DEAD, "X" },
                                { EXIT_ZOMBIE, "Z" },
                                { TASK_PARKED, "P" },
                                { TASK_DEAD, "I" }) :
                  "R",

                __entry->prev_state & TASK_REPORT_MAX ? "+" : "",
                __entry->next_comm, __entry->next_pid, __entry->next_prio)
);

```

## Eventos disponibles
Los eventos disponibles se agrupan por subsistemas y pueden consultarse en el fichero `available_events`

```bash
root@mcn:/sys/kernel/tracing# head -10 available_events 
tls:tls_device_offload_set
tls:tls_device_decrypted
tls:tls_device_rx_resync_send
tls:tls_device_rx_resync_nh_schedule
tls:tls_device_rx_resync_nh_delay
tls:tls_device_tx_resync_req
tls:tls_device_tx_resync_send
hda_intel:azx_suspend
hda_intel:azx_resume
hda_intel:azx_runtime_suspend
```

La informacion de los distintos eventos esta disponible dentro de la ruta `/sys/kernel/tracing/events` organizada por subsistemas.

```bash
root@mcn:/sys/kernel/tracing# ls ./events/
alarmtimer        enable          i2c            ksm           neigh           resctrl                  thp
amd_cpu           error_report    i915           kvm           net             rpm                      timer
avc               exceptions      initcall       kvmmmu        netlink         rseq                     tlb
block             ext4            intel_iommu    libata        nmi             rtc                      tls
bpf_test_run      fib             interconnect   lock          notifier        rv                       udp
bpf_trace         fib6            iocost         mac80211      oom             sched                    v4l2
bridge            filelock        iomap          mac80211_msg  osnoise         scsi                     vb2
cfg80211          filemap         iommu          maple_tree    page_isolation  sd                       vmalloc
cgroup            fs_dax          io_uring       mce           pagemap         signal                   vmscan
clk               ftrace          ipi            mctp          page_pool       skb                      vsyscall
compaction        gpio            irq            mdio          percpu          smbus                    watchdog
context_tracking  handshake       irq_matrix     mei           power           sock                     wbt
cpuhp             hda             irq_vectors    migrate       printk          spi                      workqueue
cros_ec           hda_controller  iwlwifi        mmap          pwm             swiotlb                  writeback
csd               hda_intel       iwlwifi_data   mmap_lock     qdisc           sync_trace               x86_fpu
dev               header_event    iwlwifi_io     mmc           ras             syscalls                 xdp
devfreq           header_page     iwlwifi_msg    module        raw_syscalls    task                     xen
devlink           huge_memory     iwlwifi_ucode  mptcp         rcu             tcp                      xhci-hcd
dma_fence         hwmon           jbd2           msr           regmap          thermal
drm               hyperv          kmem           napi          regulator       thermal_power_allocator
```

## Habilitar eventos
Para habilitar un evento se escribe 1 en el fichero enable de su directorio


```bash
root@mcn:/sys/kernel/tracing# echo nop > current_tracer
root@mcn:/sys/kernel/tracing# echo 1 > events/sched/sched_switch/enable 

root@mcn:/sys/kernel/tracing# head -20 trace
# tracer: nop
#
# entries-in-buffer/entries-written: 84862/638662   #P:4
#
#                                _-----=> irqs-off/BH-disabled
#                               / _----=> need-resched
#                              | / _---=> hardirq/softirq
#                              || / _--=> preempt-depth
#                              ||| / _-=> migrate-disable
#                              |||| /     delay
#           TASK-PID     CPU#  |||||  TIMESTAMP  FUNCTION
#              | |         |   |||||     |         |
          <idle>-0       [002] d..2.  9739.522821: sched_switch: prev_comm=swapper/2 prev_pid=0 prev_prio=120 prev_state=R ==> next_comm=sudo next_pid=10967 next_prio=120
            sudo-10967   [002] d..2.  9739.522872: sched_switch: prev_comm=sudo prev_pid=10967 prev_prio=120 prev_state=S ==> next_comm=io.elementary.t next_pid=10851 next_prio=120
 io.elementary.t-10851   [002] d..2.  9739.522910: sched_switch: prev_comm=io.elementary.t prev_pid=10851 prev_prio=120 prev_state=S ==> next_comm=swapper/2 next_pid=0 next_prio=120
          <idle>-0       [002] d..2.  9739.522926: sched_switch: prev_comm=swapper/2 prev_pid=0 prev_prio=120 prev_state=R ==> next_comm=sudo next_pid=10967 next_prio=120
            sudo-10967   [002] d..2.  9739.522979: sched_switch: prev_comm=sudo prev_pid=10967 prev_prio=120 prev_state=S ==> next_comm=swapper/2 next_pid=0 next_prio=120
          <idle>-0       [002] d..2.  9739.523015: sched_switch: prev_comm=swapper/2 prev_pid=0 prev_prio=120 prev_state=R ==> next_comm=sudo next_pid=10967 next_prio=120
            sudo-10967   [002] d..2.  9739.523069: sched_switch: prev_comm=sudo prev_pid=10967 prev_prio=120 prev_state=S ==> next_comm=kworker/u8:3 next_pid=27791 next_prio=120
    kworker/u8:3-27791   [002] d..2.  9739.523071: sched_switch: prev_comm=kworker/u8:3 prev_pid=27791 prev_prio=120 prev_state=I ==> next_comm=swapper/2 next_pid=0 next_prio=120
```
Se puede ver cada linea insertada con el formato indicado en el `TP_printk` del `TRACE_EVENT`

## Deshabilitar eventos
Para deshabilitar un evento basta con escribir 0 en el fichero `enable` de su directorio.

```bash
root@mcn:/sys/kernel/tracing# echo 0 > events/sched/sched_switch/enable 
```

## Habilitar/Deshabilitar todos los eventos de un subsistema
Es posible habilitar o deshabilitar todos los eventos de un subsistema escribiendo el valor adecuado en el fichero `enable` del subsistema

```bash
root@mcn:/sys/kernel/tracing# echo 0 > events/sched/enable 
```

## Consultar estado del evento 
Consultando el estado del fichero `enable` se puede saber si el evento esta habilitado.

Para el fichero `enable` de un subsistema tenemos las siguientes opciones:
* 0: Todos los eventos deshabilitados
* 1: Todos los eventos habilitados
* X: Algunos eventos habilitados

## Contenido de un evento
Que podemos encontrar dentro del directorio de un evento

```bash
root@mcn:/sys/kernel/tracing# ls events/sched/sched_switch/ 
enable  filter  format  hist  id  inject  trigger
```

### enable
habilitador del evento
### id
id del eveto. Es unico para cada evento del Kernel
### format
formato del evento. Esta relacionado con la informacion registrada en el `TRACE_EVENT` de definicion
```bash
root@mcn:/sys/kernel/tracing# cat events/sched/sched_switch/format 
name: sched_switch
ID: 330
format:
	field:unsigned short common_type;	offset:0;	size:2;	signed:0;
	field:unsigned char common_flags;	offset:2;	size:1;	signed:0;
	field:unsigned char common_preempt_count;	offset:3;	size:1;	signed:0;
	field:int common_pid;	offset:4;	size:4;	signed:1;

	field:char prev_comm[16];	offset:8;	size:16;	signed:0;
	field:pid_t prev_pid;	offset:24;	size:4;	signed:1;
	field:int prev_prio;	offset:28;	size:4;	signed:1;
	field:long prev_state;	offset:32;	size:8;	signed:1;
	field:char next_comm[16];	offset:40;	size:16;	signed:0;
	field:pid_t next_pid;	offset:56;	size:4;	signed:1;
	field:int next_prio;	offset:60;	size:4;	signed:1;

print fmt: "prev_comm=%s prev_pid=%d prev_prio=%d prev_state=%s%s ==> next_comm=%s next_pid=%d next_prio=%d", REC->prev_comm, REC->prev_pid, REC->prev_prio, (REC->prev_state & ((((0x00000000 | 0x00000001 | 0x00000002 | 0x00000004 | 0x00000008 | 0x00000010 | 0x00000020 | 0x00000040) + 1) << 1) - 1)) ? __print_flags(REC->prev_state & ((((0x00000000 | 0x00000001 | 0x00000002 | 0x00000004 | 0x00000008 | 0x00000010 | 0x00000020 | 0x00000040) + 1) << 1) - 1), "|", { 0x00000001, "S" }, { 0x00000002, "D" }, { 0x00000004, "T" }, { 0x00000008, "t" }, { 0x00000010, "X" }, { 0x00000020, "Z" }, { 0x00000040, "P" }, { 0x00000080, "I" }) : "R", REC->prev_state & (((0x00000000 | 0x00000001 | 0x00000002 | 0x00000004 | 0x00000008 | 0x00000010 | 0x00000020 | 0x00000040) + 1) << 1) ? "+" : "", REC->next_comm, REC->next_pid, REC->next_prio

```
* Nombre del evento
* Id
* Parametros comnes para todos los eventos
* Parametros definidos para el evento. Para los parametros se indica su tipo, nombre, tamaño y offset.
* Formato del evento en el buffer derivado del `TP_printk`
### filter
Permite filtrar los casos para los que se trazara un evento activo. Es posible definir expreiones logicas con los parametros del evento para indicar los casos en lo que se dejara traza.

#### Añadir un filtro
```bash
root@mcn:/sys/kernel/tracing# echo 'prev_comm == "bash" && prev_state & 0x02' > events/sched/sched_switch/filter
root@mcn:/sys/kernel/tracing# echo 1 > events/sched/sched_switch/enable

root@mcn:/sys/kernel/tracing# cat events/sched/sched_switch/filter 
prev_comm == "bash" && prev_state & 0x02

root@mcn:/sys/kernel/tracing# echo > trace
root@mcn:/sys/kernel/tracing# cat trace
# tracer: nop
#
# entries-in-buffer/entries-written: 3/3   #P:4
#
#                                _-----=> irqs-off/BH-disabled
#                               / _----=> need-resched
#                              | / _---=> hardirq/softirq
#                              || / _--=> preempt-depth
#                              ||| / _-=> migrate-disable
#                              |||| /     delay
#           TASK-PID     CPU#  |||||  TIMESTAMP  FUNCTION
#              | |         |   |||||     |         |
           <...>-9646    [000] d..2.  2720.358172: sched_switch: prev_comm=bash prev_pid=9646 prev_prio=120 prev_state=D ==> next_comm=swapper/0 next_pid=0 next_prio=120
           <...>-9438    [001] d..2.  2748.022592: sched_switch: prev_comm=bash prev_pid=9438 prev_prio=120 prev_state=D ==> next_comm=kworker/1:1H next_pid=120 next_prio=100
            bash-10668   [003] d..2.  2951.299949: sched_switch: prev_comm=bash prev_pid=10668 prev_prio=120 prev_state=D ==> next_comm=swapper/3 next_pid=0 next_prio=120

```
#### Borrar los filtos
```bash
root@mcn:/sys/kernel/tracing# echo 0 > events/sched/sched_switch/filter
```
Es posible borrar todos los filtros de un subsistema escribiendo `0` en el fichero `filter` del subsistem

#### Operadores validos
* Campos de tipo numerico: ==, !=, <, <=, >, >=, &
* Campos de tipo cadena: ==, !=, ~ (admite comodines *?)

### Trigger
Permite configurar acciones que se ejecutaran cuando sucede un evento.
* Apagar el trazado
* Encender el trazado
* Tomar una instantanea
* Generar un volcado de pila
* Habilitar otro evento
* Deshabilitar otro evento

#### Añadir un trigger
```bash
# echo 'command[:count] [if filter]' > trigger
```

#### Eliminar un trigger
```bash
# echo '!command[:count] [if filter]' > trigger
```
Se puede omitir el filtro para desactivarlo

#### Comandos del trigger
##### Enable_event
Permite habilitar otro evento cuando se dispara el trigger para un evento

Formato
```bash
enable_event:<system>:<event>[:count]
```

Ejemplo
```bash
root@mcn:/sys/kernel/tracing# echo 'enable_event:kmem:kmalloc:1' > events/syscalls/sys_enter_read/trigger
```

Eliminar el trigger
```bash
root@mcn:/sys/kernel/tracing# echo '!enable_event:kmem:kmalloc:1' > events/syscalls/sys_enter_read/trigger
```

##### Disable_event
Permite deshabilitar un evento cuando se dispara el trigger para un evento

Formato
```bash
disable_event:<system>:<event>[:count]
```

Ejemplo  
```bash
root@mcn:/sys/kernel/tracing# echo 'disable_event:kmem:kmalloc' > events/syscalls/sys_exit_read/trigger
```

Eliminar el trigger  
```bash
root@mcn:/sys/kernel/tracing# echo '!disable_event:kmem:kmalloc' > events/syscalls/sys_exit_read/trigger
```

##### stacktrace
Realiza un volcalo del call stack cuando se dispara el evento

Formato
```bash
stacktrace[:count]
```

Ejemplo  
```bash
root@mcn:/sys/kernel/tracing# echo 'stacktrace:5 if bytes_req >= 65536' > events/kmem/kmalloc/trigger
```

Eliminar el trigger  
```bash
root@mcn:/sys/kernel/tracing# echo '!stacktrace:5' > events/kmem/kmalloc/trigger
```

##### hist
Añade estadisticas para generar un histrograma en base a los eventos

Formato
```bash
hist:keys=<field1[,field2,...]>[:values=<field1[,field2,...]>]
  [:sort=<field1[,field2,...]>][:size=#entries][:pause][:continue]
  [:clear][:name=histname1] [if <filter>]
```
Cuando el trigger se dispara, se añade una entrada a la tabla histograma usando los keys y values definidos. Si no se definen values se hara un conteo de las veces que sucede.


Ejemplos
```bash
root@mcn:/sys/kernel/tracing# echo 'hist:key=common_pid.execname' > events/raw_syscalls/sys_enter/trigger
```

```bash
root@mcn:/sys/kernel/tracing# echo 'hist:key=common_pid.execname' > events/raw_syscalls/sys_enter/trigger
```

##### Comandos disponibles
Es posible ver los comandos disponibles mostrando el contenido del fichero `trigger`
```bash
root@mcn:/sys/kernel/tracing# cat events/kmem/kmalloc/trigger 
# Available triggers:
# traceon traceoff snapshot stacktrace enable_event disable_event enable_hist disable_hist hist
```

### hist
Es el fichero donde ser almacenan el histrograma recogido mediante el commando `hist` aplicado al `trigger` de un evento
```bash
root@mcn:/sys/kernel/tracing# cat events/raw_syscalls/sys_enter/hist 
# event histogram
#
# trigger info: hist:keys=common_pid.execname:vals=hitcount:sort=hitcount:size=2048 [active]
#

{ common_pid: kerneloops      [      2044] } hitcount:          2
{ common_pid: kerneloops      [      2046] } hitcount:          2
[...]
{ common_pid: Compositor      [      3228] } hitcount:       7250
{ common_pid: IPC I/O Child   [      5107] } hitcount:       7782
{ common_pid: gdbus           [      2478] } hitcount:       8141
{ common_pid: bluetooth       [      5786] } hitcount:       8692
{ common_pid: IPC I/O Parent  [      3176] } hitcount:      11404
{ common_pid: gala            [      2699] } hitcount:      13358
{ common_pid: Xorg            [      1220] } hitcount:      14757
{ common_pid: Timer           [      5116] } hitcount:      20771
{ common_pid: firefox         [      3081] } hitcount:      28517
```

### inject

## Habilitar eventos con `set_event`
Todos los eventos listados en `available_events` pueden tambien habilitarse añadiendolos al fichero `set_event`

```bash
root@mcn:/sys/kernel/tracing# echo sched:sched_wakeup > set_event
```

Es posible añadir mas eventos mediante el uso de `>>`. Para deshabilitarlo se puede usar la negacion con `!` o deshabilitarlos todos con:
```bash
root@mcn:/sys/kernel/tracing# echo > set_event
```