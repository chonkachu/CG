# Computer-Graphics
SCC0650 Computer Graphics ICMC-USP Computer Science Bachelor's

Para rodar o programa (Ubuntu 22.04):
1. Baixe dependências **sudo apt-get install libglfw3-dev libglm-dev mesa-utils libglu1-mesa-dev freeglut3-dev mesa-common-dev**
2. Compile o GLEW para sua máquina seguindo este [link](https://github.com/nigels-com/glew?tab=readme-ov-file#build)
3. Após a compilação, mova **libGLEW.so, libGLEW.so.2.2, libGLEW.so.2.2.0** gerados na pasta lib/ para /usr/lib/
4. Rode o Makefile e execute ./main

Selecionando as teclas de 0..9 podemos selecionar algum dos objetos e utilizar transformações nele
1. 0..1 -> Estrelas
2. 2..4 -> Arbustos
3. 5..7 -> Nuvens
4. 8 -> Casa
5. 9 -> Nave

Comandos de transformação:
1. Z,C Rotação (Z para 2D, Y para 3D)
2. J,K Escala
3. W,A,S,D translação
