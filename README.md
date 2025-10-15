# Linux Kernel Programming Cheatsheet

[![License: GPL v2](https://img.shields.io/badge/License-GPL%20v2-blue.svg)](https://www.gnu.org/licenses/old-licenses/gpl-2.0.en.html)

Este repositorio son solo mis apuntes tomados mientras estudio y practico desarrollando drivers para el kernel de Linux. Son solo una guía rápida para mi yo del futuro. Todo lo que está reflejado está basado en pruebas reales, aunque puede contener alguna errata fruto de la edición.

## 📚 Documentación

### [Módulos del Kernel](./modulos.md)
Guía sobre desarrollo de módulos del kernel de Linux:
- Configuración y tipos de módulos
- Comandos básicos (`lsmod`, `modinfo`, `insmod`, `rmmod`)
- Estructura básica de módulos
- Paso de parámetros (`module_param`)
- Símbolos y exportación
- Proceso de compilación

### [Debug y Logging](./debug.md)
Técnicas de debugging y logging en el kernel:
- Buffer de mensajes del kernel (`dmesg`)
- Funciones de logging (`printk`, `pr_*`)
- Dynamic Debug
- Kernel panic y oops
- Herramientas de depuración

## 🛠️ Herramientas

### [Plantillas de Módulos](./module_template/)
Generador automático de módulos del kernel basico partiendo de plantillas predefinidas:
- **`generate_module.py`** - Script Python para generar módulos automáticamente
- **`template.c`** - Plantilla de módulo usado como punto de partido
- **`Makefile`** - Makefile genérico para desarrollo, compilacion y test de los modulos

## 📖 Referencias Útiles

- [Documentación oficial del Kernel](https://www.kernel.org/doc/html/latest/)
- [Linux Device Drivers, 3rd Edition](https://lwn.net/Kernel/LDD3/)
- [Kernel Newbies](https://kernelnewbies.org/)

## 📄 Licencia

Este proyecto está bajo la Licencia GNU General Public License v2.0. Ver el archivo [LICENSE](LICENSE) para más detalles.