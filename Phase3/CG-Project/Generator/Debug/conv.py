from sys import argv
from random import randint, seed

seed() # Gerar seed para o módulo random.

if len(argv) >= 2:
    try:
        
        fd = open(argv[1], 'r')
        fw = open("pontosGL.txt", 'w')
        print(f'File found with name {argv[1]}. Converting...')

        for line in fd.readlines():

            # fw.write(f'\nglColor3f({randint(0,1)},{randint(0,1)},{randint(0,1)});\n\n') # Cor aleatória.

            points = line.split(',') # Obter os pontos do triângulo.
            points[-1] = points[-1].strip() # Remover new line.

            for coords in points:

                fw.write(f'glVertex3f({coords.replace(" ",",")});\n') # Glut point notation.


        print("\nConversion done.")

    except FileNotFoundError as e:
        print("File does not exist!")
        

    except Exception as e:
        print("Some bad things happened.")
        print(f'Error explanation: {e.args}')

else:
    print("File not provided.")



