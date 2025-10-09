# Linux Kernel Debug Cheatsheet

## Índice
- [Linux Kernel Debug Cheatsheet](#linux-kernel-debug-cheatsheet)
  - [Índice](#índice)
  - [El buffer circular de mensajes del kernel (dmesg)](#el-buffer-circular-de-mensajes-del-kernel-dmesg)
    - [Ver mensajes](#ver-mensajes)
    - [Borrar el buffer](#borrar-el-buffer)
    - [Borrarlo sin representarlo](#borrarlo-sin-representarlo)
    - [No imprimir timestamps](#no-imprimir-timestamps)
    - [Imprimir timestamps human ready](#imprimir-timestamps-human-ready)
    - [Imprimir solo mensajes con cierto nivel de prioridad](#imprimir-solo-mensajes-con-cierto-nivel-de-prioridad)
    - [Imprimir prioridad de los mensajes al inicio del mensaje](#imprimir-prioridad-de-los-mensajes-al-inicio-del-mensaje)
  - [Tamaño del log](#tamaño-del-log)
  - [Memory dumps](#memory-dumps)
    - [Volcado del stack](#volcado-del-stack)
  - [Mensajes Oops](#mensajes-oops)
  - [Volcados condicionales](#volcados-condicionales)
    - [BUG() o BUG\_ON(condition)](#bug-o-bug_oncondition)
    - [WARN() o WARN\_ON(condition)](#warn-o-warn_oncondition)
  - [Log de mensajes (printk(...))](#log-de-mensajes-printk)
    - [Niveles de log actuales](#niveles-de-log-actuales)
    - [Cambiar niveles de log](#cambiar-niveles-de-log)
  - [Log de mensajes (pr\_\*(...))](#log-de-mensajes-pr_)
    - [Activacion de pr\_debug(...)](#activacion-de-pr_debug)
  - [Dynamic Debug](#dynamic-debug)
    - [Sintaxis](#sintaxis)
    - [Acciones:](#acciones)
    - [Filtros](#filtros)
  - [Estandarizando el formato de logs para el modulo](#estandarizando-el-formato-de-logs-para-el-modulo)
  - [Logs en disco (syslogd)](#logs-en-disco-syslogd)
  - [Logs en disco (systemd-journal)](#logs-en-disco-systemd-journal)
  - [Kernel logs desde espacio de usuario](#kernel-logs-desde-espacio-de-usuario)
  - [Configuraciones recomendadas en el Kernel para debug](#configuraciones-recomendadas-en-el-kernel-para-debug)

## El buffer circular de mensajes del kernel (dmesg)

### Ver mensajes
```c
$ sudo dmesg
```
### Borrar el buffer
Borrarlo tras representarlo
```c
$ sudo dmesg -c
```

### Borrarlo sin representarlo
```c
$ sudo dmesg -C
```

### No imprimir timestamps
```c
$ sudo dmesg -t
```

### Imprimir timestamps human ready
```c
$ sudo dmesg -T
```

### Imprimir solo mensajes con cierto nivel de prioridad
```c
$ sudo dmesg -l err,warn
```

### Imprimir prioridad de los mensajes al inicio del mensaje
```c
$ sudo dmesg -x
```

## Tamaño del log
- Definido en make menuconfig -> General Setup -> Kernel log buffer size
- size __LOG_BUF_LEN bytes where __LOG_BUF_LEN equals (1 << CONFIG_LOG_BUF_SHIFT).
```bash
$ cat /boot/config-`uname -r` | grep CONFIG_LOG_BUF_SHIFT
CONFIG_LOG_BUF_SHIFT=18
```
- dmesg reads by default a buffer of max 16392 bytes, so if you use a larger logbuffer you have to invoke dmesg with the -s parameter

## Memory dumps

### Volcado del stack
```c
static int myinit(void)
{
	pr_info("dump_stack myinit\n");
	dump_stack();
	pr_info("dump_stack after\n");
	return 0;
}
```
```bash
$ sudo dmesg
...
[11528.715980] dump_stack myinit
[11528.715987] CPU: 3 PID: 52913 Comm: insmod Tainted: G           OE      6.8.0-83-generic #83~22.04.1-Ubuntu
[11528.715993] Hardware name: ENZ C16B/C16B, BIOS c 11/21/2014
[11528.715996] Call Trace:
[11528.715999]  <TASK>
[11528.716002]  dump_stack_lvl+0x76/0xa0
[11528.716012]  ? __pfx_myinit+0x10/0x10 [dump_stack]
[11528.716017]  dump_stack+0x10/0x20
[11528.716022]  myinit+0x1a/0xfc0 [dump_stack]
[11528.716026]  do_one_initcall+0x5e/0x340
[11528.716036]  do_init_module+0x97/0x290
[11528.716043]  load_module+0xb85/0xcd0
[11528.716048]  ? security_kernel_post_read_file+0x75/0x90
[11528.716056]  init_module_from_file+0x96/0x100
[11528.716060]  ? init_module_from_file+0x96/0x100
[11528.716066]  idempotent_init_module+0x11c/0x310
[11528.716071]  __x64_sys_finit_module+0x64/0xd0
[11528.716075]  x64_sys_call+0x15ed/0x2480
[11528.716079]  do_syscall_64+0x81/0x170
[11528.716084]  ? ksys_mmap_pgoff+0x120/0x270
[11528.716088]  ? syscall_exit_to_user_mode+0x83/0x260
[11528.716092]  ? do_syscall_64+0x8d/0x170
[11528.716096]  ? ksys_read+0x73/0x100
[11528.716100]  ? generic_fillattr+0x4a/0x160
[11528.716103]  ? _copy_to_user+0x25/0x50
[11528.716107]  ? cp_new_stat+0x143/0x180
[11528.716111]  ? __do_sys_newfstatat+0x53/0x90
[11528.716115]  ? syscall_exit_to_user_mode+0x83/0x260
[11528.716119]  ? do_syscall_64+0x8d/0x170
[11528.716123]  ? irqentry_exit+0x43/0x50
[11528.716126]  ? exc_page_fault+0x94/0x1b0
[11528.716129]  entry_SYSCALL_64_after_hwframe+0x78/0x80
[11528.716133] RIP: 0033:0x79d21251e8fd
[11528.716145] Code: 5b 41 5c c3 66 0f 1f 84 00 00 00 00 00 f3 0f 1e fa 48 89 f8 48 89 f7 48 89 d6 48 89 ca 4d 89 c2 4d 89 c8 4c 8b 4c 24 08 0f 05 <48> 3d 01 f0 ff ff 73 01 c3 48 8b 0d 03 b5 0f 00 f7 d8 64 89 01 48
[11528.716148] RSP: 002b:00007ffff75c4cf8 EFLAGS: 00000246 ORIG_RAX: 0000000000000139
[11528.716151] RAX: ffffffffffffffda RBX: 0000642a4e29d780 RCX: 000079d21251e8fd
[11528.716153] RDX: 0000000000000000 RSI: 0000642a39c08cd2 RDI: 0000000000000003
[11528.716155] RBP: 0000000000000000 R08: 0000000000000000 R09: 0000000000000000
[11528.716156] R10: 0000000000000003 R11: 0000000000000246 R12: 0000642a39c08cd2
[11528.716159] R13: 0000642a4e2a16f0 R14: 0000642a39c07888 R15: 0000642a4e29d890
[11528.716162]  </TASK>
[11528.716163] dump_stack after
```

## Mensajes Oops
Un OOPS es similar a un "segfault" en espacio de usuario. El kernel lanza un mensaje OOPS cuando ocurre una excepción, como acceder a una ubicación de memoria inválida en el código del kernel.

Cuando ocurre un OOPS, el kernel realiza las siguientes operaciones:
- Mata el proceso que causó el error
- Imprime información útil para que los desarrolladores puedan depurar
- Continúa la ejecución.
    Nota: Después de un OOPS, el sistema no puede considerarse confiable, ya que algunos locks o estructuras pueden no haberse limpiado correctamente.

Un mensaje OOPS contiene la siguiente información:
- Estado del procesador
- Contenido de los registros de la CPU en el momento de la excepción
- Stack trace (traza de pila)
- Call Trace (traza de llamadas)

```c
...
static int test_oops_init(void)
{
    printk(KERN_INFO"%s: In init\n", __func__);
	//we are trying to access invalid memory location
    *(int *)0x12 = 'a';
    return 0;
}
...
```
```shell
$ $ sudo insmod oops_test.ko
Terminado (killed)

$ sudo dmesg
...
[12192.266816] test_oops_init: In init
[12192.266822] BUG: kernel NULL pointer dereference, address: 0000000000000012
[12192.266826] #PF: supervisor write access in kernel mode
[12192.266828] #PF: error_code(0x0002) - not-present page
[12192.266830] PGD 0 P4D 0 
[12192.266833] Oops: 0002 [#1] PREEMPT SMP PTI
[12192.266836] CPU: 3 PID: 56664 Comm: insmod Tainted: G           OE      6.8.0-83-generic #83~22.04.1-Ubuntu
[12192.266838] Hardware name: ENZ C16B/C16B, BIOS c 11/21/2014
[12192.266840] RIP: 0010:test_oops_init+0x1e/0x40 [oops_test]
[12192.266844] Code: 90 90 90 90 90 90 90 90 90 90 90 90 0f 1f 44 00 00 55 48 c7 c6 98 90 a3 c1 48 c7 c7 54 90 a3 c1 48 89 e5 e8 64 d4 d7 e7 31 c0 <c7> 04 25 12 00 00 00 61 00 00 00 5d 31 f6 31 ff c3 cc cc cc cc 66
[12192.266846] RSP: 0018:ffff98ee859139e0 EFLAGS: 00010246
[12192.266848] RAX: 0000000000000000 RBX: ffffffffc1a35010 RCX: 0000000000000000
[12192.266850] RDX: 0000000000000000 RSI: 0000000000000000 RDI: 0000000000000000
[12192.266851] RBP: ffff98ee859139e0 R08: 0000000000000000 R09: 0000000000000000
[12192.266852] R10: 0000000000000000 R11: 0000000000000000 R12: 0000000000000000
[12192.266854] R13: 0000000000000000 R14: ffff98ee859139f0 R15: 0000000000000000
[12192.266855] FS:  000078ad087d4c40(0000) GS:ffff88e056d80000(0000) knlGS:0000000000000000
[12192.266857] CS:  0010 DS: 0000 ES: 0000 CR0: 0000000080050033
[12192.266859] CR2: 0000000000000012 CR3: 000000010ac4e003 CR4: 00000000003706f0
[12192.266861] Call Trace:
[12192.266862]  <TASK>
[12192.266865]  ? show_regs+0x6d/0x80
[12192.266869]  ? __die+0x24/0x80
[12192.266872]  ? page_fault_oops+0x99/0x1b0
[12192.266877]  ? do_user_addr_fault+0x2f5/0x680
[12192.266880]  ? exc_page_fault+0x83/0x1b0
[12192.266884]  ? asm_exc_page_fault+0x27/0x30
[12192.266888]  ? __pfx_test_oops_init+0x10/0x10 [oops_test]
[12192.266891]  ? test_oops_init+0x1e/0x40 [oops_test]
[12192.266893]  do_one_initcall+0x5e/0x340
[12192.266898]  do_init_module+0x97/0x290
[12192.266902]  load_module+0xb85/0xcd0
[12192.266905]  ? security_kernel_post_read_file+0x75/0x90
[12192.266910]  init_module_from_file+0x96/0x100
[12192.266913]  ? init_module_from_file+0x96/0x100
[12192.266916]  idempotent_init_module+0x11c/0x310
[12192.266919]  __x64_sys_finit_module+0x64/0xd0
[12192.266922]  x64_sys_call+0x15ed/0x2480
[12192.266924]  do_syscall_64+0x81/0x170
[12192.266927]  ? syscall_exit_to_user_mode+0x83/0x260
[12192.266931]  ? do_syscall_64+0x8d/0x170
[12192.266934]  ? ksys_read+0x73/0x100
[12192.266937]  ? syscall_exit_to_user_mode+0x83/0x260
[12192.266940]  ? do_syscall_64+0x8d/0x170
[12192.266943]  ? __x64_sys_openat+0x6c/0xa0
[12192.266946]  ? syscall_exit_to_user_mode+0x83/0x260
[12192.266949]  ? do_syscall_64+0x8d/0x170
[12192.266952]  ? syscall_exit_to_user_mode+0x83/0x260
[12192.266955]  ? do_syscall_64+0x8d/0x170
[12192.266958]  ? handle_mm_fault+0xad/0x380
[12192.266960]  ? do_user_addr_fault+0x33f/0x680
[12192.266963]  ? irqentry_exit_to_user_mode+0x78/0x260
[12192.266967]  ? irqentry_exit+0x43/0x50
[12192.266969]  ? exc_page_fault+0x94/0x1b0
[12192.266973]  entry_SYSCALL_64_after_hwframe+0x78/0x80
[12192.266975] RIP: 0033:0x78ad07f1e8fd
[12192.266986] Code: 5b 41 5c c3 66 0f 1f 84 00 00 00 00 00 f3 0f 1e fa 48 89 f8 48 89 f7 48 89 d6 48 89 ca 4d 89 c2 4d 89 c8 4c 8b 4c 24 08 0f 05 <48> 3d 01 f0 ff ff 73 01 c3 48 8b 0d 03 b5 0f 00 f7 d8 64 89 01 48
[12192.266988] RSP: 002b:00007ffcde3a0b38 EFLAGS: 00000246 ORIG_RAX: 0000000000000139
[12192.266990] RAX: ffffffffffffffda RBX: 000055c303ff3780 RCX: 000078ad07f1e8fd
[12192.266992] RDX: 0000000000000000 RSI: 000055c2d58e7cd2 RDI: 0000000000000003
[12192.266993] RBP: 0000000000000000 R08: 0000000000000000 R09: 0000000000000000
[12192.266994] R10: 0000000000000003 R11: 0000000000000246 R12: 000055c2d58e7cd2
[12192.266996] R13: 000055c303ff76f0 R14: 000055c2d58e6888 R15: 000055c303ff3890
[12192.266998]  </TASK>
[12192.267000] Modules linked in: oops_test(OE+) tls xt_conntrack xt_MASQUERADE bridge stp llc xt_set ip_set nft_chain_nat nf_nat nf_conntrack nf_defrag_ipv6 nf_defrag_ipv4 xt_addrtype nft_compat nf_tables libcrc32c nfnetlink xfrm_user xfrm_algo ccm rfcomm vboxnetadp(OE) vboxnetflt(OE) vboxdrv(OE) cmac algif_hash algif_skcipher af_alg bnep overlay snd_hda_codec_realtek snd_hda_codec_generic intel_rapl_msr snd_hda_codec_hdmi uvcvideo snd_hda_intel videobuf2_vmalloc intel_rapl_common uvc x86_pkg_temp_thermal snd_intel_dspcfg intel_powerclamp snd_intel_sdw_acpi coretemp snd_hda_codec videobuf2_memops kvm_intel videobuf2_v4l2 snd_hda_core snd_hwdep mei_hdcp mei_pxp videodev btusb binfmt_misc kvm snd_pcm iwlmvm irqbypass btrtl mac80211 btintel btbcm videobuf2_common btmtk snd_seq_midi joydev snd_seq_midi_event rapl mc intel_cstate libarc4 bluetooth snd_rawmidi snd_seq iwlwifi ecdh_generic snd_seq_device snd_timer ecc input_leds cmdlinepart serio_raw spi_nor snd cfg80211 mtd at24 mei_me mei soundcore intel_pch_thermal
[12192.267062]  soc_button_array mac_hid acpi_pad sch_fq_codel msr parport_pc ppdev lp parport efi_pstore ip_tables x_tables autofs4 hid_logitech_hidpp hid_logitech_dj hid_generic usbhid hid crct10dif_pclmul spi_intel_platform spi_intel i915 crc32_pclmul polyval_clmulni polyval_generic ghash_clmulni_intel sha256_ssse3 sha1_ssse3 drm_buddy i2c_algo_bit ttm r8169 ahci drm_display_helper psmouse realtek libahci i2c_i801 cec xhci_pci xhci_pci_renesas i2c_smbus lpc_ich rc_core video wmi aesni_intel crypto_simd cryptd [last unloaded: dump_stack(OE)]
[12192.267095] CR2: 0000000000000012
[12192.267097] ---[ end trace 0000000000000000 ]---
[12192.267107] RIP: 0010:test_oops_init+0x1e/0x40 [oops_test]
[12192.267110] Code: 90 90 90 90 90 90 90 90 90 90 90 90 0f 1f 44 00 00 55 48 c7 c6 98 90 a3 c1 48 c7 c7 54 90 a3 c1 48 89 e5 e8 64 d4 d7 e7 31 c0 <c7> 04 25 12 00 00 00 61 00 00 00 5d 31 f6 31 ff c3 cc cc cc cc 66
[12192.267112] RSP: 0018:ffff98ee859139e0 EFLAGS: 00010246
[12192.267114] RAX: 0000000000000000 RBX: ffffffffc1a35010 RCX: 0000000000000000
[12192.267115] RDX: 0000000000000000 RSI: 0000000000000000 RDI: 0000000000000000
[12192.267126] RBP: ffff98ee859139e0 R08: 0000000000000000 R09: 0000000000000000
[12192.267127] R10: 0000000000000000 R11: 0000000000000000 R12: 0000000000000000
[12192.267129] R13: 0000000000000000 R14: ffff98ee859139f0 R15: 0000000000000000
[12192.267130] FS:  000078ad087d4c40(0000) GS:ffff88e056d80000(0000) knlGS:0000000000000000
[12192.267132] CS:  0010 DS: 0000 ES: 0000 CR0: 0000000080050033
[12192.267133] CR2: 0000000000000012 CR3: 000000010ac4e003 CR4: 00000000003706f0
[12192.267135] note: insmod[56664] exited with irqs disabled
```

## Volcados condicionales

### BUG() o BUG_ON(condition)
- Imprime el contenido de los registros
- Imprime la traza de la pila (Stack Trace)
- El proceso actual muere
```c
    switch (clk) {
        case CLOCK_REALTIME:
                name = "realtime\n";**
                sz = sizeof("realtime\n");
                break;
        case CLOCK_MONOTONIC:
                name = "monotonic\n";
                sz = sizeof("monotonic\n");
                break;
        ...

        default:
                BUG();
```

### WARN() o WARN_ON(condition)
- Imprime el contenido de los registros
- Imprime la traza de la pila (Stack Trace)
```c
    case IIO_VAL_FRACTIONAL:
        WARN_ON(attr->vals[i * 2] > 4294);
        tbl_val = attr->vals[i * 2] * 1000000 /
        attr->vals[i * 2 + 1];
        break;
```

## Log de mensajes (printk(...))

* KERN_EMERG	 (0): Mensajes críticos: el sistema no puede continuar.
* KERN_ALERT	 (1): Requiere atención inmediata (ej. error grave de hardware).
* KERN_CRIT	   (2): Error crítico (pérdida de datos, fallo grave del subsistema).
* KERN_ERR	   (3): Error en el dispositivo o módulo, pero el sistema continúa.
* KERN_WARNING (4): Condición de advertencia; posible problema futuro.
* KERN_NOTICE	 (5): Mensajes informativos importantes (no error).
* KERN_INFO    (6): Mensajes informativos generales (estado, inicio…).
* KERN_DEBUG   (7): Información de depuración; útil durante desarrollo.

```c
if( !mii_chip_table[i].phy_id1 ) {
        printk(KERN_INFO "%s: Unknown PHY transceiver found at address %d.\n",
                dev_name, phy_addr);
        mii_phy->phy_types = UNKNOWN;
}
```

Si no se especifica ningún prefijo (KERN_INFO, etc.), printk() utiliza por defecto el nivel DEFAULT_MESSAGE_LOGLEVEL, que suele corresponder a KERN_WARNING, nivel 4.

### Niveles de log actuales
```bash
$ cat /proc/sys/kernel/printk
4	4	1	7
```
* 4 :	current loglevel: Nivel de severidad máximo que se mostrará actualmente en la consola. En este caso, solo mensajes con nivel ≤ KERN_WARNING (4) se imprimirán.
* 4 : default loglevel: Nivel por defecto usado por printk() si no se especifica prefijo KERN_...
* 1 : minimum console loglevel : Nivel mínimo permitido para la consola. El kernel nunca reducirá el nivel de log por debajo de este valor.
* 7 : default message loglevel for kernel messages : Nivel de log asignado a nuevos mensajes cuando no hay configuración previa (por ejemplo, al arrancar). Generalmente es el valor máximo (KERN_DEBUG).

### Cambiar niveles de log
Para que se impriman en consola mensajes hasta KERN_INFO
```bash
sudo dmesg -n 7
```

## Log de mensajes (pr_*(...))
Son macros de conveniencia definidas en el kernel para simplificar el uso de printk()

Integra automáticamente con el Dynamic Debug (dynamic_debug), lo que permite activar o desactivar pr_debug() en tiempo de ejecución sin recompilar.

| Macro        | Equivalente printk()         | Descripción               | Cuándo se muestra (por defecto)     |
|:-------------|:-----------------------------|:--------------------------|:------------------------------------|
| pr_emerg()   | printk(KERN_EMERG ...)       | Error crítico, el sistema es inestable o inutilizable. | Siempre |
| pr_alert()   | printk(KERN_ALERT ...)       | Condición que requiere atención inmediata. | Siempre      |
| pr_crit()    | printk(KERN_CRIT ...)        | Error crítico de hardware o kernel. | Siempre            |
| pr_err()     | printk(KERN_ERR ...)         | Error importante, pero no fatal. | Visible con printk ≤ 3 o 4 |
| pr_warn()    | printk(KERN_WARNING ...)     | Aviso de posible problema. | Visible con printk ≤ 4       |
| pr_notice()  | printk(KERN_NOTICE ...)      | Información relevante, pero no crítica. | No visible por defecto |
| pr_info()    | printk(KERN_INFO ...)        | Información general o de estado. | No visible por defecto |
| pr_debug()   | printk(KERN_DEBUG ...)       | Mensajes de depuración detallados. | Solo si DEBUG está habilitado |

```c
pr_info("Set current limit of %s : %duA ~ %duA\n",
        cable->charger->regulator_name,
        cable->min_uA, cable->max_uA);
```
### Activacion de pr_debug(...)
Para que pr_debug() funcione, el módulo o kernel debe compilarse con la macro DEBUG definida:
```bash
make EXTRA_CFLAGS="-DDEBUG" ...
```
o en el codigo
```c
#define DEBUG
```

## Dynamic Debug
El sistema Dynamic Debug permite activar o desactivar en tiempo de ejecución los mensajes pr_debug() y dev_dbg() sin recompilar el kernel ni el módulo.
Funciona solo si el kernel se compiló con esta opción: `CONFIG_DYNAMIC_DEBUG=y`

Configurable en `Kernel hacking > printk and dmesg options > Enable dynamic printk() support`

El fichero de Dynamic Debug se encuentra en `/sys/kernel/debug/dynamic_debug/control` 

### Sintaxis
```bash
 echo '<filtro> <accion>' | sudo tee /sys/kernel/debug/dynamic_debug/control

```
### Acciones:
* `+p` Activa la impresión (print)
* `-p` Desactiva la impresión
* `+f` Muestra nombre de fichero y línea
* `+m` Muestra nombre de módulo
* `+t` Añade timestamp
* `+l` Añade número de línea
* `-fmpl`	Desactiva opciones adicionales
* `+` añade los flags indicadas
* `-` elimina los flags indicadas
* `=` establece exactamente los flags indicadas

### Filtros
* `file` Activa logs en un fichero concreto: `file my_driver.c +p`
* `module` Activa logs en un modulo: `module my_module +p`
* `func` Solo en una función específica: `func my_function +p`
* `line` Solo una linea en concreto: `line 120 +p`
* `line (rango)` En un rango de lineas: `line 120-140 +p`

Las acciones pueden conbinarse
```bash
$ echo 'module my_module func my_function +p' | sudo tee /sys/kernel/debug/dynamic_debug/control
```
## Estandarizando el formato de logs para el modulo
El posible darle un formato estandar a todos los logs del modulo declarando pr_fmt(fmt) al inicio del modulo (antes de los includes)
```c
// mi_modulo.c
#define pr_fmt(fmt) "%s:%s():%d " fmt, KBUILD_MODNAME, __func__, __LINE__

#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
[ ... ]
static int __init mi_modulo_init(void)
{
pr_info("initialized\n");
[ ... ]

```
```bash
[381534.391966] mi_modulo:mi_modulo_init():17 initialized
```

## Logs en disco (syslogd)
Para sistemas Debian
```bash
$ ll -h /var/log/syslog
-rw-r----- 1 syslog adm 109M oct  7 17:50 /var/log/syslog
```

Para sistemas Red Hat
```bash
$ /var/log/messages
```

## Logs en disco (systemd-journal)
Es el servicio de logs en disco usado en systemd. Se pueden consultar mediante `journalctl`
Auna todos los los de servicios, kernel, o aplicaciones

TODO

## Kernel logs desde espacio de usuario
Usado, por ejemplo durante test de modulos con scripts en espacio de usuario, para dejar logs en el buffer del kernel
```bash
$ sudo bash -c "echo \"test_script: @user msg 1\" > /dev/kmsg"
$ sudo dmesg | tail -n1
[10870.544904] test_script: @user msg 1
```
Es posible especificar el nivel del log mediante su valor numerico
```bash
$ sudo bash -c "echo \"<6>test_script: test msg at KERN_INFO\" > /dev/kmsg"
$ sudo dmesg -x | tail -n2
user  :warn  : [10870.544904] test_script: @user msg 1
user  :info  : [11489.861591] test_script: test msg at KERN_INFO
```

## Configuraciones recomendadas en el Kernel para debug
* CONFIG_DEBUG_KERNEL and CONFIG_DEBUG_INFO
* CONFIG_DEBUG_MISC
* Generic kernel debugging instruments:
  * CONFIG_MAGIC_SYSRQ (the magic SysRq hotkeys feature)
  * CONFIG_DEBUG_FS (the debugfs pseudo filesystem)
  * CONFIG_KGDB (kernel GDB; optional, recommended)
  * CONFIG_UBSAN (the undefined behaviour sanity checker)
  * CONFIG_KCSAN (the dynamic data race detector)
* Memory debugging:
  * CONFIG_SLUB_DEBUG
  * CONFIG_DEBUG_MEMORY_INIT
  * CONFIG_KASAN: The powerful Kernel Address Sanitizer (KASAN) memory checker
  * CONFIG_DEBUG_SHIRQ
  * CONFIG_SCHED_STACK_END_CHECK
  * CONFIG_DEBUG_PREEMPT
* Lock debugging:
  * CONFIG_PROVE_LOCKING: The very powerful lockdep feature to catch locking bugs!
  * CONFIG_LOCK_STAT.
  * CONFIG_DEBUG_ATOMIC_SLEEP.
* CONFIG_BUG_ON_DATA_CORRUPTION
* CONFIG_STACKTRACE
* CONFIG_DEBUG_BUGVERBOSE
* CONFIG_FTRACE (ftrace: Within its sub-menu, turn on at least a couple of“tracers", including the ‘Kernel Function [Graph] Tracer’)
* CONFIG_BUG_ON_DATA_CORRUPTION
* Arch-specific (shows up under “x86 Debugging” on the x86):
  * CONFIG_EARLY_PRINTK (arch-specific)
  * CONFIG_DEBUG_BOOT_PARAMS
  * CONFIG_UNWINDER_FRAME_POINTER (selects FRAME_POINTER and CONFIG_STACK_VALIDATION)