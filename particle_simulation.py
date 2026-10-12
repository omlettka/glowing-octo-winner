import pygame, sys, random
from collections import defaultdict
from math import dist, sqrt
from pygame import gfxdraw

pygame.init()
pygame.font.init()

width, height = 2000, 1400
screen = pygame.display.set_mode((width, height))
pygame.display.set_caption("Particle Simulation")

FONT = pygame.font.SysFont("Consolas", 14)

clock = pygame.time.Clock()

max_radius = 15
min_radius = 15
cell_size = 2 * max_radius
bounce_factor = 0.0
border_radius = min(width, height) * 0.9 // 2
substeps = 8

arr = [
    (1, 1),
    (1, 0),
    (1, -1),
    (0, 1),
    (0, 0),
    (0, -1),
    (-1, 1),
    (-1, 0),
    (-1, -1)
]
particles_grid = defaultdict(list)
particles = []

class vector:
    def __init__(self, x: float, y: float):
        self.x = x
        self.y = y

    def add(self, other: 'vector'):
        return vector(self.x + other.x, self.y + other.y)

    def subtract(self, other: 'vector'):
        return vector(self.x - other.x, self.y - other.y)

    def multiply(self, scalar: float):
        return vector(self.x * scalar, self.y * scalar)

    def divide(self, scalar: float):
        return vector(self.x / scalar, self.y / scalar)

    def magnitude(self):
        return sqrt(self.x ** 2 + self.y ** 2)

    def normalize(self):
        mag = self.magnitude()
        if mag == 0:
            return vector(0, 0)
        return self.divide(mag)
    
    def dist(self, other: 'vector'):
        return sqrt((self.x - other.x) ** 2 + (self.y - other.y) ** 2)

class Particle:
    def __init__(self, x: int, y: int, radius: int, color: pygame.Color):
        self.x = x
        self.y = y
        self.old_cords = vector(x, y)
        self.radius = radius
        self.color = color
        self.color = (255, 255, 255)
        #self.radius = 10
        self.velocity = vector(0, 0)
        self.acseleration = vector(0, 150)
        self.index = (x//cell_size, y//cell_size)
        particles.append(self)
        
        

    def update(self, deltaTime: float):
        self.applyForce(vector(0, 1000))
        self.stay_inside()
        self.move(1/60/substeps)
        #self.updateArr()
    
    def render(self, surface: pygame.Surface):
        gfxdraw.filled_circle(surface, int(self.x), int(self.y), self.radius, self.color)
        
    def solveCollision(self, other: 'Particle'):
        distance = dist((self.x, self.y), (other.x, other.y))
        if distance == 0 or distance < 0.01: return
        if distance < self.radius + other.radius:
            overlap = 0.9 * (distance - self.radius - other.radius)
            self_mass_index = other.radius ** 2 / (self.radius ** 2 + other.radius ** 2)
            other_mass_index = self.radius ** 2 / (self.radius ** 2 + other.radius ** 2)
            
            a = (self.x - other.x)
            b = (self.y - other.y)

            #self.old_cords.x -= overlap * (self.x - other.x) / distance * self_mass_index
            #self.old_cords.y -= overlap * (self.y - other.y) / distance * self_mass_index
            #other.old_cords.x += overlap * (self.x - other.x) / distance *other_mass_index
            #other.old_cords.y += overlap * (self.y - other.y) / distance *other_mass_index
            
            self.x -= overlap * a / distance * self_mass_index
            self.y -= overlap * b / distance * self_mass_index
                       
            other.x += overlap * a / distance *other_mass_index
            other.y += overlap * b / distance *other_mass_index

    def move(self, deltaTime: float):
        self.velocity = (self.x - self.old_cords.x, self.y - self.old_cords.y)
        velocity_damping = 5.0
        self.old_cords.x = self.x
        self.old_cords.y = self.y
        self.x += self.velocity[0] + (self.acseleration.x - self.velocity[0]*velocity_damping) * deltaTime * deltaTime
        self.y += self.velocity[1] + (self.acseleration.y - self.velocity[1]*velocity_damping) * deltaTime * deltaTime
        self.acseleration.x = 0
        self.acseleration.y = 0

    def stay_inside(self):
        to_obj = (self.x - width/2, self.y - height/2)
        dist = sqrt(to_obj[0]**2+to_obj[1]**2)
        if dist > border_radius - self.radius:
            n = (to_obj[0]/dist, to_obj[1]/dist)
            self.x = width/2 + n[0] * (border_radius - self.radius)
            self.y = height/2 + n[1] * (border_radius - self.radius)
    
    def applyForce(self, force: vector):
        self.acseleration.x+=force.x
        self.acseleration.y+=force.y


'''
for _ in range(substeps):
    update_grid()
    for every particle:
        for every neighbour particle:
            check_collision with neighbour (one way)
    for every particle:
        update {
            apply_force(gravity)
            move()
            stay_inside()
        }
    for every particle:
        

'''



def update_physycs(dt: float, substeps: int):
    subdt = dt / substeps
    for _ in range(substeps):
        update_grid()
        for particle in particles:
            i = particle.index[0]
            j = particle.index[1]
            for dc in arr:
                for particle2 in particles_grid.get((i+dc[0], j+dc[1]), ()):
                    if id(particle) < id(particle2) and particle != particle2:
                        particle.solveCollision(particle2)
        for particle in particles:
            particle.update(subdt)
        
def update_grid():
    global particles_grid
    particles_grid = defaultdict(list)
    for particle in particles:
        particle.index=(particle.x//cell_size,particle.y//cell_size)
        particles_grid[particle.index].append(particle)
        

if __name__ == "__main__":
    n = 10
    delay = 0.1
    timer = 0
    mouse_button = False
    k=0
    while True:
        deltaTime = clock.tick(60) / 1000.0

        for event in pygame.event.get():
            if event.type == pygame.QUIT:
                pygame.quit()
                sys.exit()
            if event.type == pygame.KEYDOWN:
                if event.key == pygame.K_ESCAPE:
                    pygame.quit()
                    sys.exit()
            if event.type == pygame.MOUSEBUTTONDOWN:
                mouse_button = True
            if event.type == pygame.MOUSEBUTTONUP:
                mouse_button = False
        timer += deltaTime
        if mouse_button and timer > delay:
            pos = pygame.mouse.get_pos()
            p = Particle(pos[0], pos[1], random.randint(min_radius, max_radius), pygame.Color(random.randint(0, 255), random.randint(0, 255), random.randint(0, 255)))
            k+=1
            timer = 0
        if n!=0 and timer > delay:
            for i in range(10):
                pos = vector(random.randint(width//2-0+i*5, width//2 + 1+i*5), height//2)
                radius = random.randint(min_radius, max_radius)
                color = pygame.Color(random.randint(0, 255), random.randint(0, 255), random.randint(0, 255))
                k+=1
                p = Particle(pos.x, pos.y, radius, color)
                p.applyForce(vector(0, 100))
                n-=1
                timer = 0

        update_physycs(deltaTime, substeps)
                    
            
            
        
        screen.fill((150, 150, 150))
        gfxdraw.filled_circle(screen, int(width // 2), int(height // 2), int(border_radius), (0, 0, 0))
        
        title = FONT.render(str(k), True, (0, 0, 0))
        screen.blit(title, (45, 15))
        for particle in particles:
            particle.render(screen)
            
        pygame.display.set_caption(f"Particle Simulation {clock.get_fps():.2f} FPS")
        pygame.display.flip()
