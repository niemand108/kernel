Real Address Module
===================

Este modulo muestra en /proc/cr3 la dirección física(real) a la que apunta el registro CR3 de cada proceso. La función de éste registro es apuntar a la Base del Directorio de Página(PDBR = Page Directory Base Register) de cada proceso durante su ejecución(en la cpu). Utilizando el PDBR y la dirección virtual de un proceso, podemos acceder a su tabla de paginación, lo que blablabla... En definitiva y sin complicarnos más, podemos decir que CR3 contiene la dirección real de cada proceso durante su ejecución.

Para ver el resultado:
  $ sudo cat /proc/cr3
