# -*- coding: utf-8 -*-
"""
Created on Sun Oct  4 16:41:34 2020

@authors: 
    Fernando Mora Gutierrez
    Sergio Alvaro Vargas Pérez
    Daniel Álvarez del Castillo Martínez
    Germán Eduardo Rodríguez Vázquez
"""
import os, sys
"""
Realice un programa que cree un archivo de tamaño 150 bytes, 
el contenido del archivo deben ser: 20 asteriscos, 30 gatos y puros ceros.
"""
fd = os.open("filesystem.os", os.O_RDWR|os.O_CREAT)
fileSize = os.fstat(fd).st_size
fillContent = ""
counter = 0
option = 1

if (fileSize == 0):
    s = "**###00000000\n"
    for i in range (10):
        fillContent += s
    bytes = os.write(fd,fillContent.encode())
while (option == 1 or option == 2 ):
    os.lseek(fd,0,0)
    option = int(input("Seleccione 1 si quiere modificar área de datos, 2 para modificar el área de # o 3 para salir: "))
    if(option == 1):
        colPosition = int(input("Seleccione el numero de columna a modificar: "))
        if(colPosition > 8):
            print("Sólo existen 8 columnas de dato, vuelva a intentarlo con un número de columna menor")
        else:
            content = input("Ingrese el contenido a insertar: ")
            lenContent = len(content)
            if(lenContent > 10):
                print("Contenido mayor a 10 bytes")
            else:
                for x in range (lenContent):
                    currentPos = (4+colPosition) + (x*15) #4 offset para bloque de datos y 15 para salto de línea
                    os.lseek(fd,currentPos,0)
                    os.write(fd,content[x].encode())
                counter +=1
                print("Insercion exitosa")

    elif(option == 2):
        col = int(input("Número de columna : "))
        if(col > 3):
            print("Sólo existen 3 columnas de #, vuelva a intentarlo con un número de columna menor")
        else:
            row = int(input("Número de renglón : "))
            if (row > 10):
                print("Sólo existen 10 renglones de #, vuelva a intentarlo con un número de renglón menor")
            else:
                content = input("Ingrese el contenido a insertar: ")
                if(len(content) > 1):
                    print("Contenido mayor a 1 byte")
                else:
                    currentPos = (1+col) + ((row-1)*15) #1 offset para bloque de # y 15 para salto de línea
                    os.lseek(fd,currentPos,0)
                    os.write(fd,content.encode())
                    counter += 1
                    print("Insercion exitosa")


os.lseek(fd,0,0)
os.write(fd,str(counter).encode())


os.fsync(fd)
os.lseek(fd,0,0)
print(os.read(fd,150).decode())
os.close(fd)
 



"""
En un menú, permita que la persona le diga qué conjunto de 10 bytes quiere modificar. 
Con lseek() localice la posición y modifique los datos. 
Si son más de 10 bytes a modificar, no se hace el ajuste.  
"""

