# Computer-Graphics
SCC0650 Computer Graphics

Para rodar o programa (Ubuntu 22.04):
1. Baixe dependências **sudo apt-get install libglfw3-dev libglm-dev mesa-utils libglu1-mesa-dev freeglut3-dev mesa-common-dev**
2. Compile o GLEW para sua máquina seguindo este [link](https://github.com/nigels-com/glew?tab=readme-ov-file#build)
3. Após a compilação, mova **libGLEW.so, libGLEW.so.2.2, libGLEW.so.2.2.0** gerados na pasta lib/ para /usr/lib/
4. Rode o Makefile e execute ./main

Comandos
1. 1-9 Seleciona um dos modelos
2. Z,X Rotação (eixo Y por padrão, precisa ter selecionado modelo)
3. J,K Translação em Y (precisa ter selecionado modelo)
4. W,A,S,D Movimentação da Camera 
5. Arrow keys Translação em X e Z (precisa ter selecionado modelo)
6. C, V Escala (precisa ter selecionado modelo)
7. O,I,U Desliga, Aumenta, Diminui a luz ambiente respectivamente
8. M, (virgula Teclado US) Aumenta e Diminui reflexão especular da luz ambiente
9. B, N Aumenta e Diminui reflexão difusa da luz ambiente
10. F Liga/Desliga a fonte de luz de um modelo (precisa ter alguma fonte de luz e estar selecionado)
11. E, R Aumenta e Diminui a reflexão difusa de um modelo (precisa estar selecionado)
12. T, Y Aumenta e Diminui a reflexão especular de um modelo (precisa estar selecionado)

Ordem dos modelos (teclas 1-9)
1. Lanterna (tem fonte de luz)
2. Lamparina (tem fonte de luz)
3. Gigante (tem fonte de luz)
4. Poste 1 (tem fonte de luz)
5. Poste 2 (tem fonte de luz)
6. Casa
7. Cama
8. Escultura da Vitoriosa
9. Pensador

OBS: o mesanino, a árvore e a grama podem receber as mesmas transformações que os modelos acima,
porém faltou teclas =(
