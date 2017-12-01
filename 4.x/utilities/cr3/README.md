 CR3 Module
===========

Este módulo muestra en /proc/cr3 la dirección física(real) a la que apunta el registro CR3 de cada proceso. La función de éste registro es, principalmente,  apuntar a la Base del Directorio de Página(PDBR = Page Directory Base Register). Utilizando el PDBR y la dirección virtual de un proceso podemos acceder a sus distintos niveles de las tablas de paginación y caminarlas hasta conseguir la dirección real de memoria. En definitiva y sin complicarnos más, podemos decir que CR3 contiene la dirección real que será base de las traducciónes virtuales de cada proceso.  Para más información ver este enlace [4] y [5]

![Direccionamiento virtual, cr3 y paginación](https://github.com/niemand108/kernel/blob/master/4.x/utilities/cr3/images/pagination.png)

Cómo usar
=========

  1. make
  
  2. insmod cr3.ko
  
  3. sudo cat /proc/cr3

Notas
=====

[1] Este código está basado en https://carteryagemann.com/pid-to-cr3.html

[2] Sólo se muestran los procesos padres, los hijos/hilos se omiten (comparten espacio de direcciones con el padre)

[3] Los PIDs que arrojan direcciones 0x000000000 son hilos del kernel [kthreads]

[4] http://rayseyfarth.com/asm/pdf/ch04-memory-mapping.pdf

[5] https://www.d.umn.edu/~gshute/os/address-translation.xhtml
