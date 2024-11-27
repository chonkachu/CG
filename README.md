# Computer-Graphics
SCC0650 Computer Graphics

Para rodar o programa (Ubuntu 22.04):
1. Baixe dependências **sudo apt-get install libglfw3-dev libglm-dev mesa-utils libglu1-mesa-dev freeglut3-dev mesa-common-dev**
2. Compile o GLEW para sua máquina seguindo este [link](https://github.com/nigels-com/glew?tab=readme-ov-file#build)
3. Após a compilação, mova **libGLEW.so, libGLEW.so.2.2, libGLEW.so.2.2.0** gerados na pasta lib/ para /usr/lib/
4. Rode o Makefile e execute ./main

Comandos
1. 1-5 Seleciona um dos modelos
2. Z,C Rotação (eixo Y por padrão, precisa ter selecionado modelo)
3. J,K Translação em Y (precisa ter selecionado modelo)
4. W,A,S,D Movimentação da Camera 
5. Arrow keys Translação em X e Z (precisa ter selecionado modelo)
6. C, V Escala (precisa ter selecionado modelo)
7. O,I,U Desliga, Aumenta, Diminui a luz ambiente respectivamente
8. Y, T Aumenta e Diminui reflexão especular
9. R, E Aumenta e Diminui reflexão difusa 
10. F Liga a fonte de luz de um modelo (veja abaixo)

Ordem dos modelos com luz (teclas 1-5)
1. Lanterna
2. Lampada
3. Gigante
4. Lampada de Rua 1
5. Lampada de Rua 2
