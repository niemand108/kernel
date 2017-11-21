Este modulo y su POC demuestran como una referencia a null en el kernel puede causar ejecución de código arbitrario en nuestro sistema. Es difícil saber el año en que se comenzó a explotar esta técnica, pero según un artículo de John Corbet[1] parece ser 2009. Desde entonces se ha restringido el acceso a las zonas bajas de memoria del usuario, mitigando el problema de las referencias nulas mediante configuraciones con sysctl y SELinux.

Para compilar el proyecto, siga los siguientes pasos.

  1. cd module && make
  2. cd ..
  3. cd poc && make
  4. cd ..
  
Una vez compilado, cargamos el módulo, el cual habilitará una entrada en el sistema de ficheros /proc, llamada /proc/nullderef:
  
  5. sudo insmod module/nullderef_module.ko
 
Para que el sistema nos permita utilizar la técnica de null dereference debemos permitir que el usuario tenga acceso a zonas bajas de memoria, concretamente a la dirección 0x00000000, que es la clave de esta técnica.
  
  6. sudo sysctl -w vm.mmap_min_addr="0"

También, en el caso de tenerlo habilitado, debemos desactivar temporalmente SELinux, ya que también añade medidas de seguridad para prohibir al usuario accesos a zonas bajas de memoria. 
  
  7. sudo setenforce 0

Una vez desactivadas las barreras, ya podemos accedes y escribir en la dirección 0x0. Ejecutamos el POC, que lo que hará será modificar el EIP/RIP a 0x41414141, demostrando que la ejecución de código es posible.

  8. ./proc/proc_exec

El resultado final lo podemos ver en los log del kernel:
  
  9. dmesg | tail
     
     **RIP: 0010:0x41414141** RSP:0033:0x7ffff7af8ea4


[1] https://lwn.net/Articles/342330/
