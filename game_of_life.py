import pygame, sys
from pygame import *
from pygame.locals import *
from random import randint
init()
width = display.Info().current_w
height = display.Info().current_h
window = display.set_mode((width, height), FULLSCREEN)
display.set_caption('Game of life')
clock = time.Clock()
max_fps = 120
font = pygame.font.SysFont('Arial', 30)
surface = Surface((width, height))



cell_size = 15
cwidth = width//(cell_size+1)
cheight = height//(cell_size+1)
ofx = width%(cell_size+1)//2
ofy = height%(cell_size+1)//2
generation = 0
gen_speed = max_fps //3
pause = True
frame = 0
update = True

plane = [[0 for i in range(cheight+2)] for j in range(cwidth+2)]


def render():
    for x in range(cwidth):
        for y in range(cheight):
            if plane[x+1][y+1]==0:
                draw.rect(surface, (255,255,255),(x*(cell_size+1)+ofx,y*(cell_size+1)+ofy,cell_size,cell_size))
            elif plane[x+1][y+1]==1:
                draw.rect(surface, (0,0,0),(x*(cell_size+1)+ofx,y*(cell_size+1)+ofy,cell_size,cell_size))

def live():
    
    for i in range(cwidth):
        plane[i+1][0]=plane[i+1][cheight]
        plane[i+1][cheight+1]=plane[i+1][1]
    for i in range(cheight):
        plane[0][i+1]=plane[cwidth][i+1]
        plane[cwidth+1][i+1]=plane[1][i+1]
    plane[0][0]=plane[cwidth][cheight]
    plane[cwidth+1][0]=plane[1][cheight]
    plane[cwidth+1][cheight+1]=plane[1][1]
    plane[0][cheight+1]=plane[cwidth][1]
    frozen_plane = [row[:] for row in plane[:]]
    for x in range(cwidth):
        for y in range(cheight):
            neighbours = frozen_plane[x][y]+frozen_plane[x][y+1]+frozen_plane[x][y+2]+frozen_plane[x+1][y]+frozen_plane[x+1][y+2]+frozen_plane[x+2][y]+frozen_plane[x+2][y+1]+frozen_plane[x+2][y+2]
            if plane[x+1][y+1] == 0:
                if neighbours==3:
                    plane[x+1][y+1] = 1
            elif plane[x+1][y+1] == 1:
                if neighbours <2 or neighbours >3:
                     plane[x+1][y+1] = 0


while True:
    for e in event.get():
        if e.type == QUIT:
            quit()
            sys.exit()
        if e.type == KEYUP:
            if e.key == K_ESCAPE:
                quit()
                sys.exit()
            if e.key == K_SPACE:
                if pause:
                    pause = False
                else:
                    pause = True
            if e.key == K_r:
                generation = 0
                update = True
                for x in range(cwidth):
                    for y in range(cheight):
                        
                        if randint(0,8)>5:
                            plane[x+1][y+1]=1
                        else:
                            plane[x+1][y+1]=0
            if e.key ==K_c:
                generation = 0
                update = True
                plane = [[0 for i in range(cheight+2)] for j in range(cwidth+2)]
        if e.type == MOUSEWHEEL:
            gen_speed -=e.y
            if gen_speed <1:
                gen_speed = 1
            if gen_speed > 300:
                gen_speed = 300
    pressed = mouse.get_pressed()
    pos = mouse.get_pos()
    if pressed[0]:
        update = True
        plane[(pos[0]-ofx)//(cell_size+1)+1][(pos[1]-ofy)//(cell_size+1)+1]=1
    elif pressed[2]:
        update = True
        plane[(pos[0]-ofx)//(cell_size+1)+1][(pos[1]-ofy)//(cell_size+1)+1]=0



    if not pause:
        frame +=1
        if frame%gen_speed==0:
            update = True
            generation+=1
            live()


    if update:
        render()
    
    fps = str(int(clock.get_fps()))
    font = pygame.font.SysFont('Arial', 30)
    clock.tick(max_fps)
    window.blit(surface, (0, 0))
    window.blit(font.render(fps, False, (234,123,56)), (ofx,ofy))
    window.blit(font.render(str(generation), False, (14,153,65)), (ofx,30+ofy))
    display.flip()
    update = False
