#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <windows.h>
#include <sys/time.h>
#include <math.h>


void imprimir_array_resultado();
void array_fill();

#define ZOOM 0.9

double microsegundos() { /* obtiene la hora del sistema en microsegundos */
    struct timeval t;
    if (gettimeofday(&t, NULL) < 0 )
        return 0.0;
    return (t.tv_usec + t.tv_sec * 1000000.0);
}

//variables globales para facil aceso
typedef struct coordenadas{
    int x;
    int y;
} cords;

typedef struct peso_coordenadas{
    int x;
    int y;
    unsigned int peso;
} pes_cords;

typedef struct monticulo_minimos{
    int current_size;
    int vector_size;
    pes_cords *vector;
} * p_monticulo;

struct data{
    int width;
    int height;
    int *pixels;
}* frame;

unsigned int promesa(pes_cords);

p_monticulo mont_ini;
p_monticulo mont_fin;
pes_cords coli1;
pes_cords coli2;
unsigned char ** pesos;
char ** visitados;
char ** viene_de;

enum {
    NADA = 0,
    ARRIBA = 1,
    ABAJO = 2,
    DERECHA = 3,
    IZQUIERDA = 4,
};

enum {
    NADAP = NADA,
    ARRIBAP = DERECHA,
    ABAJOP = IZQUIERDA,
    DERECHAP = ARRIBA,
    IZQUIERDAP = ABAJO,
};

cords inicio;
cords final;
int size_x;
int size_y;

int tam;
int comprobados = 0;
int anadidos = 0;
unsigned int mayor = 0;

int mostrar_proceso;
int delay;
int iteraciones;
int cont;
int cont2;
int caminos;
int color;
int error;

int * quitflag;


char *keyboard;


struct mouse_dat{
    int x, y;
    int buttons;
}* mouse;

enum { MOUSE_LEFT = 0b1, MOUSE_MIDDLE = 0b10, MOUSE_RIGHT = 0b100, MOUSE_X1 = 0b1000, MOUSE_X2 = 0b10000, MOUSE_SCROLL_UP = 0b100000, MOUSE_SCROLL_DOWN = 0b1000000};

typedef struct{
    int quit;
    int nCmdShow;
    int sizez_x;
    int size_y;
    unsigned char ** pesos;
    char ** visitados;
    char ** viene_de;
    HINSTANCE hInstance;
    struct data *frame;
    char * keyboard;
    struct mouse_dat * mouse;
} thread_args;

void imprimir_tamano(unsigned long int size){
    tam = 0;

    while (size >= 1000){
        size /= 1000;
        tam += 1;
    }

    printf("%lu", size);

    switch (tam) {
        case 0:
            printf("B");
            break;
        case 1:
            printf("KB");
            break;
        case 2:
            printf("MB");
            break;
        case 3:
            printf("GB");
            break;
        default:
            printf("DF");
    }
}

void ** ini_array(int endSize){
    void ** ary;
    ary = malloc(size_x * sizeof(void *));
    for(int i = 0; i < size_x; i++) {
        ary[i] = malloc(size_y * endSize);
    }

    return ary;
}


void free_array(char ** ary){
    for(int i = 0; i < size_x; i++){
        free(ary[i]);
    }

    free(ary);
}

void fill_arrs(){
    for(int i = 0, j; i < size_x; i++)
        for(j = 0; j < size_y; j++) {
            visitados[i][j] = 0;
            viene_de[i][j] = 0;
        }
}

void array_custom(){
    printf("\nIntroduce los valores:\n");
    for(int i = 0, j; i < size_x; i++)
        for(j = 0; j < size_y; j++) {
            printf("(%i, %i): ", i, j);
            scanf("%i", &pesos[i][j]);
        }
}

void array_binario(){
    printf("\nIntroduce los dos valores:\n");
    int a,b, aux;
    scanf("%i %i", &a, &b);

    for(int i = 0, j; i < size_x; i++)
        for(j = 0; j < size_y; j++){
            aux = rand() % 2;
            if(aux)
                pesos[i][j] = a;
            else
                pesos[i][j] = b;
        }
}

void array_cuadratico(){
    unsigned char aux;

    for(int i = 0, j; i < size_x; i++)
        for(j = 0; j < size_y; j++) {
            aux = rand() % 0x10 + 1;
            if(aux > 1)
                pesos[i][j] = aux * aux - 1;
            else
                pesos[i][j] = 1;
        }
}

void array_cuadratico_inv(){
    unsigned char aux;

    for(int i = 0, j; i < size_x; i++)
        for(j = 0; j < size_y; j++) {
            aux = rand() % 0x10 + 1;
            if(aux < 0x10)
                pesos[i][j] = 0xff - aux * aux;
            else
                pesos[i][j] = 1;
        }
}

void array_senosuidal(){
    printf("\nIntroduce el factor x, y y general\n");
    double x,y, fac;
    char res;
    scanf("%lf %lf %lf", &x, &y, &fac);

    for(int i = 0, j; i < size_x; i++)
        for(j = 0; j < size_y; j++) {
            res = ((sin( 3 * M_PI * i / size_x * x) * sin(3 * M_PI * j / size_y * y) + 1) * 128 * fac);
            if(!res)
                res = 1;
            pesos[i][j] = res;
        }
}

void array_senosuidal_circular(){
    printf("\nIntroduce la cantidad de puntos\n");
    int cantidad;
    scanf("%i", &cantidad);

    double * arrx = malloc(cantidad * sizeof(double));
    double * arry = malloc(cantidad * sizeof(double));
    double * arrf = malloc(cantidad * sizeof(double));
    double res;
    unsigned char resaux;

    for(int i = 0; i < cantidad; i++) {
        arrx[i] = (rand() / (double) RAND_MAX) * 2 * M_PI;
        arry[i] = (rand() / (double) RAND_MAX) * 2 * M_PI;
        arrf[i] = (rand() / (double) RAND_MAX) * min(size_x,size_y) / 100;
    }


    for(int i = 0, j, aux; i < size_x; i++)
        for(j = 0; j < size_y; j++) {
            res = 0;
            for (aux = 0; aux < cantidad; aux++)
                res += cos(arrf[aux] * sqrt( pow( (i / (double) size_x) * 2 * M_PI - arrx[aux],2) + pow( (j / (double) size_y)  * 2 * M_PI - arry[aux],2))) / cantidad;
            resaux = 128 * (res + 1);

            if(!resaux)
                resaux = 1;

            pesos[i][j] = resaux;
        }



    free(arrx);
    free(arry);
    free(arrf);
}

void array_montanoso(){
    printf("\nIntroduce la cantidad de montanas\n");
    int montanas;
    scanf("%i", &montanas);

    printf("\nAnadir montanas en las esquinas?\n");
    int esquinas;
    scanf("%i", &esquinas);

    esquinas = !!esquinas; //convierte esquinas a 0 o 1

    double * arrx = malloc((montanas + esquinas * 4) * sizeof(double));
    double * arry = malloc((montanas + esquinas * 4) * sizeof(double));
    double res, maxi;
    unsigned char resaux;

    for(int i = 0; i < montanas; i++) {
        arrx[i] = (rand() / (double) RAND_MAX) * 10;
        arry[i] = (rand() / (double) RAND_MAX) * 10;
    }

    arrx[montanas + 0] = 0;
    arrx[montanas + 1] = 10;
    arrx[montanas + 2] = 0;
    arrx[montanas + 3] = 10;


    arry[montanas + 0] = 0;
    arry[montanas + 1] = 0;
    arry[montanas + 2] = 10;
    arry[montanas + 3] = 10;

    //averiguar cual es el valor máximo que se alcanza
    maxi = 0;
    for(int i = 0, j; i < montanas + esquinas * 4; i++) {
        res = 0;
        for (j = 0; j < montanas + esquinas * 4; j++) {
            res += pow(M_E, -(sqrt(pow(arrx[j] - arrx[i], 2) + pow(arry[j] - arry[i], 2))));
        }
        if(maxi < res)
            maxi = res;
    }

    for(int i = 0, j, aux; i < size_x; i++)
        for(j = 0; j < size_y; j++) {
            res = 0;
            for (aux = 0; aux < montanas + esquinas * 4; aux++)
                res += pow(M_E,-(sqrt( pow( (i / (double) size_x) * 10 - arrx[aux],2) + pow( (j / (double) size_y) * 10 - arry[aux],2))));
            resaux = (res * 255 / maxi) + 1;


            pesos[i][j] = resaux;
        }



    free(arrx);
    free(arry);

}


void array_valles(){
    printf("\nIntroduce la cantidad de valles\n");
    int valles;
    scanf("%i", &valles);

    double * arrx = malloc((valles) * sizeof(double));
    double * arry = malloc((valles) * sizeof(double));
    double res, maxi;
    unsigned char resaux;

    for(int i = 0; i < valles; i++) {
        arrx[i] = (rand() / (double) RAND_MAX) * 10;
        arry[i] = (rand() / (double) RAND_MAX) * 10;
    }

    //averiguar cual es el valor máximo que se alcanza
    maxi = 0;
    for(int i = 0, j; i < valles; i++) {
        res = 0;
        for (j = 0; j < valles; j++) {
            res += pow(M_E, -(sqrt(pow(arrx[j] - arrx[i], 2) + pow(arry[j] - arry[i], 2))));
        }
        if(maxi < res)
            maxi = res;
    }

    for(int i = 0, j, aux; i < size_x; i++)
        for(j = 0; j < size_y; j++) {
            res = 0;
            for (aux = 0; aux < valles; aux++)
                res += pow(M_E,-(sqrt( pow( (i / (double) size_x) * 10 - arrx[aux],2) + pow( (j / (double) size_y) * 10 - arry[aux],2))));
            resaux = 256 - ((res * 255 / maxi) + 1);


            pesos[i][j] = resaux;
        }



    free(arrx);
    free(arry);

}


//todo fix aleatoridad
void array_perlin(){
    printf("\nIntroduce la profundidad del ruido\n");
    int profundidad;
    scanf("%i", &profundidad);

    //reservar memoria para los numeros aleatorios_factor
    double * aleatorios_desplazamientox = malloc(profundidad * sizeof(double));
    double * aleatorios_desplazamientoy = malloc(profundidad * sizeof(double));
    double * aleatorios_factor = malloc(profundidad * sizeof(double));

    double ** resultados = (double **) ini_array(sizeof(double));

    for(int i = 0; i < profundidad; i++){
        aleatorios_desplazamientox[i] = rand() / (double)RAND_MAX;
        aleatorios_desplazamientoy[i] = rand() / (double)RAND_MAX;
        aleatorios_factor[i] = rand()/(double)RAND_MAX;
    }

    double sum;
    double mini = 0;
    double maxi = 0;
    char res;

    //guardar valores y encontrar factores de escalado
    for(int i = 0, j, aux; i < size_x; i++)
        for(j = 0; j < size_y; j++){

            //res va a ser la suma de la ecuación
            sum = 0;
            for(aux = 0; aux < profundidad; aux++){
                sum += sin(( 2 * M_PI * i * (aux + 1) / size_x + 2 * M_PI * aleatorios_desplazamientox[aux])) * sin((2 * M_PI * j * (aux + 1) / size_y + 2 * M_PI * aleatorios_desplazamientoy[aux])) * sqrt(aleatorios_factor[aux]) / pow(2, aux + 2);
            }
            if(sum > maxi)
                maxi = sum;

            if(sum < mini)
                mini = sum;

            resultados[i][j] = sum;
        }


    //rellenar array de resultados
    for(int i = 0, j; i < size_x; i++)
        for(j = 0; j < size_y; j++)
            pesos[i][j] = ((resultados[i][j] + mini) / (maxi + mini) * 254) + 1;

    for(int i = 0, j; i < size_x; i++)
        free(resultados[i]);
    free(resultados);

    free(aleatorios_desplazamientox);
    free(aleatorios_desplazamientoy);
    free(aleatorios_factor);
}

void array_bloques(){
    printf("\nIntroduce el tamano de la grid en x, y, la frecuencia de los bloques y el valor con el que rellenar:\n");
    int x, y, val;
    double frec;
    scanf("%i %i %lf %i", &x, &y, &frec, &val);




    //rellenar array original
    array_fill();

    //rellenar el array a bloques
    for(int i = 0, j, auxx,auxy; i < x; i++)
        for(j = 0; j < y; j++)
            if(rand()/(double)RAND_MAX <= frec) {
                for(auxx = i * size_x / x; auxx < (i+1) * size_x / x; auxx++)
                    for(auxy = j * size_y / y; auxy < (j+1) * size_y / y; auxy++)
                        pesos[auxx][auxy] = val;
            }


}

cords buscar_visitado_con_vecinos(unsigned char ** arr, int ladox, int ladoy){
    cords res;
    res.x = -1;
    res.y = -1;

    for(int i = 0, j; i < ladox; i++)
        for(j = 0; j < ladoy; j++){
            if(arr[i][j]){
                if((i+1 < ladox && !arr[i+1][j]) || (i-1 >= 0 && !arr[i-1][j]) || (j+1 < ladoy && !arr[i][j+1]) || (j-1 >= 0 && !arr[i][j-1])){
                    res.x = i;
                    res.y = j;
                    return res;
                }
            }
        }


    return res;
}
void array_laberinto1(){
    printf("\nIntroduzca el lado del pasillo\n");
    int lado;
    scanf("%i", &lado);
    int x = size_x / lado + 1,y = size_x / lado + 1;
    int visit_x = x / 2, visit_y = y / 2;

    //asignar memoria
    unsigned char ** arr = malloc(x * sizeof(unsigned char *));
    unsigned char ** visit = malloc(visit_x * sizeof(unsigned char *));
    for(int i = 0; i < x; i++)
        arr[i] = malloc(y * sizeof(unsigned char));
    for(int i = 0; i < visit_x; i++)
        visit[i] = malloc(visit_y * sizeof(unsigned char));


    //rellenar el array con ff o 00 intermitentemente
    for(int i = 0, j; i < x; i++)
        for(j = 0; j < y; j++)
            if(i % 2 || j % 2)
                arr[i][j] = 0xff;
            else
                arr[i][j] = 0x01;

    //rellenar visit con 0
    for(int i = 0, j; i < visit_x; i++)
        for(j = 0; j < visit_y; j++)
            visit[i][j] = 0;


    //se han rellenado todos los nodos con 00 y pasillos con ff, ahora hay que liberar algunos pasillos
    cords local;
    local.x = 0;
    local.y = 0;
    visit[0][0] = 1;

    //ir rellenando de forma aleatoria el array hasta que se reciban, coordenadas negativas
    while(local.x >= 0){
        switch (rand() % 4) {
            case 1:
                if(local.x + 1 < visit_x && !visit[local.x + 1][local.y]){
                    //actualizar visit
                    visit[local.x + 1][local.y] = 1;

                    //actualizar el camino
                    arr[2*local.x + 1][2*local.y] = 0x01;

                    //actualizar local
                    local.x += 1;
                }else
                    //se ha fallado en añadir un nuevo camino, buscar un candidato válido
                    local = buscar_visitado_con_vecinos(visit, visit_x, visit_y);
                break;
            case 2:
                if(local.x - 1 >= 0 && !visit[local.x - 1][local.y]){
                    //actualizar visit
                    visit[local.x - 1][local.y] = 1;

                    //actualizar el camino
                    arr[2*local.x - 1][2*local.y] = 0x01;

                    //actualizar local
                    local.x -= 1;
                }else
                    //se ha fallado en añadir un nuevo camino, buscar un candidato válido
                    local = buscar_visitado_con_vecinos(visit, visit_x, visit_y);
                break;
            case 3:
                if(local.y + 1 < visit_y && !visit[local.x][local.y + 1]){
                    //actualizar visit
                    visit[local.x][local.y + 1] = 1;

                    //actualizar el camino
                    arr[2*local.x][2*local.y + 1] = 0x01;

                    //actualizar local
                    local.y += 1;
                }else
                    //se ha fallado en añadir un nuevo camino, buscar un candidato válido
                    local = buscar_visitado_con_vecinos(visit, visit_x, visit_y);
                break;
            default:
                if(local.y - 1 >= 0 && !visit[local.x][local.y - 1]){
                    //actualizar visit
                    visit[local.x][local.y - 1] = 1;

                    //actualizar el camino
                    arr[2*local.x][2*local.y - 1] = 0x01;

                    //actualizar local
                    local.y -= 1;
                }else
                    //se ha fallado en añadir un nuevo camino, buscar un candidato válido
                    local = buscar_visitado_con_vecinos(visit, visit_x, visit_y);
                break;
        }
    }

    //se ha rellenado el array de forma satisfactoria, ahora rellenear pesos
    for(int i = 0, j; i < size_x; i++)
        for(j = 0; j < size_y; j++)
            pesos[i][j] = arr[i/lado][j/lado];


    //liberar memoria
    for(int i = 0; i < x; i++)
        free(arr[i]);
    free(arr);

    for(int i = 0; i < visit_x; i++)
        free(visit[i]);
    free(visit);
}

void array_laberinto2(){
    printf("\nIntroduzca el lado del pasillo\n");
    int lado;
    scanf("%i", &lado);
    int x = size_x / lado + 1,y = size_x / lado + 1;
    int visit_x = x / 2, visit_y = y / 2;

    //asignar memoria
    unsigned char ** arr = malloc(x * sizeof(unsigned char *));
    unsigned char ** visit = malloc(visit_x * sizeof(unsigned char *));
    for(int i = 0; i < x; i++)
        arr[i] = malloc(y * sizeof(unsigned char));
    for(int i = 0; i < visit_x; i++)
        visit[i] = malloc(visit_y * sizeof(unsigned char));


    //rellenar el array con ff o 00 intermitentemente
    for(int i = 0, j; i < x; i++)
        for(j = 0; j < y; j++)
            if(i % 2 || j % 2)
                arr[i][j] = 0xff;
            else
                arr[i][j] = 0x01;

    //rellenar visit con 0
    for(int i = 0, j; i < visit_x; i++)
        for(j = 0; j < visit_y; j++)
            visit[i][j] = 0;


    //se han rellenado todos los nodos con 00 y pasillos con ff, ahora hay que liberar algunos pasillos
    cords local;
    local.x = 0;
    local.y = 0;
    visit[0][0] = 1;

    //ir rellenando de forma aleatoria el array hasta que se reciban, coordenadas negativas
    while(local.x >= 0){
        switch (rand() % 4) {
            case 1:
                if(local.x + 1 < visit_x && !visit[local.x + 1][local.y]){
                    //actualizar visit
                    visit[local.x + 1][local.y] = 1;

                    //actualizar el camino
                    arr[2*local.x + 1][2*local.y] = 0x01;

                    //actualizar local
                    local.x += 1;
                }else
            case 2:
                if(local.x - 1 >= 0 && !visit[local.x - 1][local.y]){
                    //actualizar visit
                    visit[local.x - 1][local.y] = 1;

                    //actualizar el camino
                    arr[2*local.x - 1][2*local.y] = 0x01;

                    //actualizar local
                    local.x -= 1;
                }else
            case 3:
                if(local.y + 1 < visit_y && !visit[local.x][local.y + 1]){
                    //actualizar visit
                    visit[local.x][local.y + 1] = 1;

                    //actualizar el camino
                    arr[2*local.x][2*local.y + 1] = 0x01;

                    //actualizar local
                    local.y += 1;
                }else
            default:
                if(local.y - 1 >= 0 && !visit[local.x][local.y - 1]){
                    //actualizar visit
                    visit[local.x][local.y - 1] = 1;

                    //actualizar el camino
                    arr[2*local.x][2*local.y - 1] = 0x01;

                    //actualizar local
                    local.y -= 1;
                }else
                    //se ha fallado en añadir un nuevo camino, buscar un candidato válido
                    local = buscar_visitado_con_vecinos(visit, visit_x, visit_y);
                break;
        }
    }

    //se ha rellenado el array de forma satisfactoria, ahora rellenear pesos
    for(int i = 0, j; i < size_x; i++)
        for(j = 0; j < size_y; j++)
            pesos[i][j] = arr[i/lado][j/lado];


    //liberar memoria
    for(int i = 0; i < x; i++)
        free(arr[i]);
    free(arr);

    for(int i = 0; i < visit_x; i++)
        free(visit[i]);
    free(visit);
}


void array_laberinto3(){
    printf("\nIntroduzca el lado del pasillo\n");
    int lado;
    scanf("%i", &lado);
    int x = size_x / lado + 1,y = size_x / lado + 1;
    int visit_x = x / 2, visit_y = y / 2;

    //asignar memoria
    unsigned char ** arr = malloc(x * sizeof(unsigned char *));
    unsigned char ** visit = malloc(visit_x * sizeof(unsigned char *));
    for(int i = 0; i < x; i++)
        arr[i] = malloc(y * sizeof(unsigned char));
    for(int i = 0; i < visit_x; i++)
        visit[i] = malloc(visit_y * sizeof(unsigned char));


    //rellenar el array con ff o 00 intermitentemente
    for(int i = 0, j; i < x; i++)
        for(j = 0; j < y; j++)
            if(i % 2 || j % 2)
                arr[i][j] = 0xff;
            else
                arr[i][j] = 0x01;

    //rellenar visit con 0
    for(int i = 0, j; i < visit_x; i++)
        for(j = 0; j < visit_y; j++)
            visit[i][j] = 0;


    //se han rellenado todos los nodos con 00 y pasillos con ff, ahora hay que liberar algunos pasillos
    cords local;
    local.x = visit_x / 2;
    local.y = visit_y / 2;
    visit[local.x][local.y] = 1;

    //hacer un vector que contenga las posiciones desde la primera hasta el final
    cords * cams = malloc(visit_x * visit_y * sizeof(cords));
    int pntr = 0;

    //ir rellenando de forma aleatoria el array hasta que se reciban, coordenadas negativas
    while(local.x >= 0){
        switch (rand() % 4) {
            case 1:
                if(local.x + 1 < visit_x && !visit[local.x + 1][local.y]){
                    //actualizar visit
                    visit[local.x + 1][local.y] = 1;

                    //actualizar el camino
                    arr[2*local.x + 1][2*local.y] = 0x01;

                    //actualizar caminos
                    cams[pntr] = local;
                    pntr++;

                    //actualizar local
                    local.x += 1;
                }else
                    //se ha fallado en añadir un nuevo camino, buscar un candidato válido
                    if(!pntr)
                        local = buscar_visitado_con_vecinos(visit, visit_x, visit_y);
                    else{
                        pntr--;
                        local = cams[pntr];
                    }

                break;
            case 2:
                if(local.x - 1 >= 0 && !visit[local.x - 1][local.y]){
                    //actualizar visit
                    visit[local.x - 1][local.y] = 1;

                    //actualizar el camino
                    arr[2*local.x - 1][2*local.y] = 0x01;

                    //actualizar caminos
                    cams[pntr] = local;
                    pntr++;

                    //actualizar local
                    local.x -= 1;
                }else
                    //se ha fallado en añadir un nuevo camino, buscar un candidato válido
                if(!pntr)
                    local = buscar_visitado_con_vecinos(visit, visit_x, visit_y);
                else{
                    pntr--;
                    local = cams[pntr];
                }
                break;
            case 3:
                if(local.y + 1 < visit_y && !visit[local.x][local.y + 1]){
                    //actualizar visit
                    visit[local.x][local.y + 1] = 1;

                    //actualizar el camino
                    arr[2*local.x][2*local.y + 1] = 0x01;

                    //actualizar caminos
                    cams[pntr] = local;
                    pntr++;

                    //actualizar local
                    local.y += 1;
                }else
                    //se ha fallado en añadir un nuevo camino, buscar un candidato válido
                if(!pntr)
                    local = buscar_visitado_con_vecinos(visit, visit_x, visit_y);
                else{
                    pntr--;
                    local = cams[pntr];
                }
                break;
            default:
                if(local.y - 1 >= 0 && !visit[local.x][local.y - 1]){
                    //actualizar visit
                    visit[local.x][local.y - 1] = 1;

                    //actualizar el camino
                    arr[2*local.x][2*local.y - 1] = 0x01;

                    //actualizar caminos
                    cams[pntr] = local;
                    pntr++;

                    //actualizar local
                    local.y -= 1;
                }else
                    //se ha fallado en añadir un nuevo camino, buscar un candidato válido
                if(!pntr)
                    local = buscar_visitado_con_vecinos(visit, visit_x, visit_y);
                else{
                    pntr--;
                    local = cams[pntr];
                }
                break;
        }
    }

    //se ha rellenado el array de forma satisfactoria, ahora rellenear pesos
    for(int i = 0, j; i < size_x; i++)
        for(j = 0; j < size_y; j++)
            pesos[i][j] = arr[i/lado][j/lado];


    //liberar memoria
    for(int i = 0; i < x; i++)
        free(arr[i]);
    free(arr);

    for(int i = 0; i < visit_x; i++)
        free(visit[i]);
    free(visit);

    free(cams);
}


void array_pseudolaberinto(){
    printf("\nIntroduzca el lado del pasillo y la densidad del laberinto\n");
    int lado;
    double densidad;
    scanf("%i %lf", &lado, &densidad);
    int x = size_x / lado + 1,y = size_y / lado + 1;

    unsigned char ** arr = malloc(x * sizeof(unsigned char *));
    for(int i = 0; i < x; i++)
        arr[i] = malloc(y * sizeof(unsigned char));


    //rellenar el array con ff o 01 intermitentemente con el añadido de que tansolo el 0,5 de las paredes están cubiertas
    for(int i = 0, j; i < x; i++)
        for(j = 0; j < y; j++)
            if(i % 2 || j % 2){
                arr[i][j] = 0xff;

            }else
                arr[i][j] = 0x01;

    //liberar aleatoriamente el 50% de los caminos
    for(int i = 0, j; i < x; i++)
        for(j = 0; j < y; j++)
            if((j+i)%2)
                if(rand()/(double)RAND_MAX >= densidad)
                    arr[i][j] = 0x01;


    //rellenar el array original de manera acorde
    for(int i = 0, j; i < size_x; i++)
        for(j = 0; j < size_y; j++)
            pesos[i][j] = arr[i/lado][j/lado];


    //liberar memoria
    for(int i = 0; i < x; i++)
        free(arr[i]);
    free(arr);
}

void array_random(){
    printf("\nIntroduce el valor minimo y maximo:\n");
    int min, lmax, aux;
    scanf("%i %i", &min, &lmax);

    //get el maximo y minimo de verdad
    if( lmax < min){
        aux = min;
        min = lmax;
        lmax = aux;
    }

    //aux va a ser la diferencia
    aux = lmax - min;

    for(int i = 0, j; i < size_x; i++)
        for(j = 0; j < size_y; j++)
            pesos[i][j] = (unsigned char) rand() % aux + min;
}

unsigned char limit(int val){
    if(val <= 0)
        return 1;
    if(val > 255)
        return 255;
    return val;
}

int ruido_cont(int ruido){
    if(ruido)
        return (rand() % ruido) - ruido / 2;
    return 0;
}

unsigned char cond_diamante(int x, int y, int lenx, int leny, int ruido){

    if(y + leny >= size_y)
        return limit((pesos[x + lenx][y] + pesos[x - lenx][y] + pesos[x][y - leny]) / 3 + ruido_cont(ruido));
    if(x + lenx >= size_x)
        return limit((pesos[x - lenx][y] + pesos[x][y - leny] + pesos[x][y + leny]) / 3 + ruido_cont(ruido));
    if(y - leny < 0)
        return limit((pesos[x + lenx][y] + pesos[x - lenx][y] + pesos[x][y + leny]) / 3 + ruido_cont(ruido));
    if(x - lenx < 0)
        return limit((pesos[x + lenx][y] + pesos[x][y - leny] + pesos[x][y + leny]) / 3 + ruido_cont(ruido));


    return limit((pesos[x + lenx][y] + pesos[x - lenx][y] + pesos[x][y - leny] + pesos[x][y + leny]) / 4 + ruido_cont(ruido));
}

void diamante_recursivo(cords TL, cords BR, int ruido){

    if(TL.x == BR.x - 1 || TL.x == BR.x)
        if(TL.y == BR.y - 1 || TL.y == BR.y)
            return;


    cords mid;
    mid.x = (TL.x + BR.x)/2;
    mid.y = (TL.y + BR.y)/2;

    pesos[mid.x][mid.y] = limit((pesos[TL.x][TL.y] + pesos[TL.x][BR.y] + pesos[BR.x][TL.y] + pesos[BR.x][BR.y])/4 + ruido_cont(ruido));

    pesos[mid.x][BR.y] = cond_diamante(mid.x, BR.y, mid.x - TL.x, mid.y - TL.y, ruido);
    pesos[mid.x][TL.y] = cond_diamante(mid.x, TL.y, mid.x - TL.x, mid.y - TL.y, ruido);
    pesos[BR.x][mid.y] = cond_diamante(BR.x, mid.y, mid.x - TL.x, mid.y - TL.y, ruido);
    pesos[TL.x][mid.y] = cond_diamante(TL.x, mid.y, mid.x - TL.x, mid.y - TL.y, ruido);

    diamante_recursivo(TL, mid, ruido / 2);
    diamante_recursivo(mid, BR, ruido / 2);

    cords T;
    T.x = mid.x;
    T.y = TL.y;

    cords R;
    R.x = BR.x;
    R.y = mid.y;

    diamante_recursivo(T, R, ruido / 2);

    cords L;
    L.x = TL.x;
    L.y = mid.y;

    cords B;
    B.x = mid.x;
    B.y = BR.y;

    diamante_recursivo(L, B, ruido / 2);
}

void array_diamante(){
    printf("\nIntroduce fuerza del ruido:\n");
    int ruido;
    scanf("%i", &ruido);

    pesos[0][0] = limit(rand() & 0xff);
    pesos[0][size_y - 1] = limit(rand() & 0xff);
    pesos[size_x - 1][0] = limit(rand() & 0xff);
    pesos[size_x - 1][size_y - 1] = limit(rand() & 0xff);

    cords TL;
    TL.x = 0;
    TL.y = 0;

    cords BR;
    BR.x = size_x - 1;
    BR.y = size_y - 1;

    diamante_recursivo(TL, BR, ruido);
}

void array_fill(){
    printf("\nQue tipo de array quieres?");
    printf("\n0: Aleatorio");
    printf("\n1: Custom");
    printf("\n2: Binario");
    printf("\n3: Cuadratico");
    printf("\n4: Cuadratico neg");
    printf("\n5: Senosuidal");
    printf("\n6: Senosuidal circular");
    printf("\n7: Montanoso");
    printf("\n8: Con valles");
    printf("\n9: Suave");
    printf("\n10: Bloques");
    printf("\n11: Pseudolaberinto");
    printf("\n12: Laberinto generado de izquierda a derecha de arriba a abajo (lento)");
    printf("\n13: Laberinto generado desde el principo (lento)");
    printf("\n14: Laberinto generado desde el punto (lento)");
    printf("\n15: Ruido de diamante");



    printf("\n\n");
    int val;
    scanf("%i", &val);


    printf("\nGenerando array...");
    switch (val) {
        case 1:
            array_custom();
            break;
        case 2:
            array_binario();
            break;
        case 3:
            array_cuadratico();
            break;
        case 4:
            array_cuadratico_inv();
            break;
        case 5:
            array_senosuidal();
            break;
        case 6:
            array_senosuidal_circular();
            break;
        case 7:
            array_montanoso();
            break;
        case 8:
            array_valles();
            break;
        case 9:
            array_perlin();
            break;
        case 10:
            array_bloques();
            break;
        case 11:
            array_pseudolaberinto();
            break;
        case 12:
            array_laberinto1();
            break;
        case 13:
            array_laberinto2();
            break;
        case 14:
            array_laberinto3();
            break;
        case 15:
            array_diamante();
            break;
        default:
            array_random();

    }
}

void flotar(p_monticulo mont, int indice){
    pes_cords aux;
    while (indice > 0 && mont->vector[(indice-1)/2].peso + promesa(mont->vector[(indice-1)/2]) >= mont->vector[indice].peso + promesa(mont->vector[indice])){
        aux = mont->vector[(indice-1)/2];
        mont->vector[(indice-1)/2] = mont->vector[indice];
        mont->vector[indice] = aux;
        indice = (indice - 1)/2;
    }
}

void introducir_en_monticulo(p_monticulo mont, pes_cords value){
    if(mont->vector_size <= mont->current_size + 1){
        if(error)
            printf("\nMonticulo se quedo sin memoria, reasignando memoria\n");
        pes_cords * aux = malloc(mont->vector_size * 2 * sizeof(pes_cords));
        for(int i = 0; i < mont->current_size; i++)
            aux[i] = mont->vector[i];
        mont->vector_size *= 2;
        free(mont->vector);
        mont->vector = aux;
        if(error) {
            printf("\nMonticulo reasignado con ");
            unsigned long int tamano = mont->vector_size * sizeof(pes_cords);
            imprimir_tamano(tamano);
            printf(" de memoria\n");
        }
    }


    mont->vector[mont->current_size] = value;
    flotar(mont, mont->current_size);
    mont->current_size++;
}

void hundir(p_monticulo mont, int indice){
    int der, izq, aux_index;
    pes_cords aux;
    do {
        der = 2 * indice + 2;
        izq = 2 * indice + 1;
        aux_index = indice;
        if(der < mont->current_size && mont->vector[der].peso + promesa(mont->vector[der]) < mont->vector[indice].peso + promesa(mont->vector[indice]))
            indice = der;
        if(izq < mont->current_size && mont->vector[izq].peso + promesa(mont->vector[izq]) < mont->vector[indice].peso + promesa(mont->vector[indice]))
            indice = izq;

        aux = mont->vector[indice];
        mont->vector[indice] = mont->vector[aux_index];
        mont->vector[aux_index] = aux;
    } while (aux_index != indice);
}

pes_cords sacar_cabeza(p_monticulo mont){
    pes_cords aux = mont->vector[0];
    mont->current_size--;
    mont->vector[0] = mont->vector[mont->current_size];
    hundir(mont, 0);
    return aux;
}

void print_arr_esp(unsigned char ** arr){
    HANDLE handle = GetStdHandle(STD_OUTPUT_HANDLE);
    unsigned char last_type = 0;
    for(int i = 0, j; i < size_x; i++) {
        for (j = 0; j < size_y; j++) {

            if(color && last_type != arr[i][j]) {
                SetConsoleTextAttribute(handle, arr[i][j]);
                last_type = arr[i][j];
            }
            printf("%02X", arr[i][j]);
        }
        if(color && last_type != 0x7) {
            SetConsoleTextAttribute(handle, 0x7);
            last_type = 0x7;
        }
        if(size_x > 100)
            printf("  - %i", i);

        printf("\n");
    }
    //devolver la consola a su estado original
    if(color)
        SetConsoleTextAttribute(handle, 0x7);
}

void print_mont(p_monticulo mont){
    for(int i = 0; i < mont->current_size; i++){
        printf("%i: (%i, %i, %i)", i, mont->vector[i].peso, mont->vector[i].x, mont->vector[i].y);
        printf("\n");
    }
}

int is_in_bounds(int x, int y){
    return x >= 0 && x < size_x && y >= 0 && y < size_y;
}

pes_cords nuevo_valor(int x, int y,unsigned int peso){
    pes_cords ret;
    ret.peso = peso;
    ret.x = x;
    ret.y = y;

    return ret;
}

char tipo_de_adyacencia(int x, int y){
    if(x) {
        if (x > 0)
            return ARRIBA;
        else
            return ABAJO;
    }else if(y){
        if (y > 0)
            return DERECHA;
        else
            return IZQUIERDA;
    }else
        return NADA;


}

pes_cords inv_tipo_de_adyacencia(int tipo){
    switch (tipo) {
        case ARRIBA:
            return nuevo_valor(1,0,0);
        case ABAJO:
            return nuevo_valor(-1,0,0);
        case DERECHA:
            return nuevo_valor(0,1,0);
        case IZQUIERDA:
            return nuevo_valor(0,-1,0);
        default:
            return nuevo_valor(0,0,0);
    }
}

pes_cords buscar_peso_en_loc_por_mont(p_monticulo mont, int x, int y){
    for(int i = 0; i < mont->current_size; i++)
        if(mont->vector[i].x == x && mont->vector[i].y == y)
            return mont->vector[i];


    return nuevo_valor(0,0,0);
}



void cond_repetida(p_monticulo mont, pes_cords pos, pes_cords prov, char valor_visita){
    if(is_in_bounds(pos.x, pos.y) && !visitados[pos.x][pos.y]) {
        visitados[pos.x][pos.y] = valor_visita;
        pos.peso += pesos[pos.x][pos.y];
        introducir_en_monticulo(mont, pos);
        anadidos++;
        viene_de[pos.x][pos.y] = tipo_de_adyacencia(prov.x - pos.x, prov.y - pos.y);
        if(pos.peso > mayor)
            mayor = pos.peso;
    } else if(is_in_bounds(pos.x, pos.y) && visitados[pos.x][pos.y] + valor_visita == 0){
        //si se culpe esta condición significa que ha habido una colisión

        //asignarle a pos un nuevo peso
        //pos.peso = pesos_desde_origen[pos.x][pos.y];
        if(visitados[pos.x][pos.y] == 1)
            pos = buscar_peso_en_loc_por_mont(mont_ini, pos.x, pos.y);
        else
            pos = buscar_peso_en_loc_por_mont(mont_fin, pos.x, pos.y);


        if(pos.peso + prov.peso < coli1.peso + coli2.peso){
            //si se llega aqí se ha encontrado un nuevo camino mínimo
            coli1 = pos;
            coli2 = prov;
        }
    }
}

void meter_vecinos(p_monticulo mont, pes_cords pos){
    cond_repetida(mont, nuevo_valor(pos.x + 1, pos.y, pos.peso), pos, visitados[pos.x][pos.y]);
    cond_repetida(mont, nuevo_valor(pos.x - 1, pos.y, pos.peso), pos, visitados[pos.x][pos.y]);

    cond_repetida(mont, nuevo_valor(pos.x, pos.y + 1, pos.peso), pos, visitados[pos.x][pos.y]);
    cond_repetida(mont, nuevo_valor(pos.x, pos.y - 1, pos.peso), pos, visitados[pos.x][pos.y]);

    //duplicar el valor en visitados para asignarlo como recorrido
    visitados[pos.x][pos.y] *= 2;
    comprobados++;
}

unsigned int promesa(pes_cords dat){
        if(visitados[dat.x][dat.y] > 0){
            return abs(final.x - dat.x) + abs(final.y - dat.y);
        }else{
            return abs(dat.x - inicio.x) + abs(dat.y - inicio.y);
        }
}


//todo fix promesa
void buscar_camino_de_esquina_a_esquina(){
    cont = 0;
    cont2 = 0;

    //inicialización
    pes_cords aux;
    //la esquina de arriba izquierda ya está visitada por el grupo 1
    visitados[inicio.x][inicio.y] = 1;
    mont_ini->current_size = 1;
    mont_ini->vector[0].x = inicio.x;
    mont_ini->vector[0].y = inicio.y;
    mont_ini->vector[0].peso = pesos[inicio.x][inicio.y];

    //la esquina de abajo derecha ya está visitada por el grupo -1
    visitados[final.x][final.y] = -1;
    mont_fin->current_size = 1;
    mont_fin->vector[0].x = final.x;
    mont_fin->vector[0].y = final.y;
    mont_fin->vector[0].peso = pesos[final.x][final.y];

    //asignarle a coli1 y 2 los valores máximos como placeholders, como se suman mas tarde, es necesario dividirlos entre 2
    coli1.peso = INT_MAX/2;
    coli1.x = inicio.x;
    coli1.y = inicio.y;

    coli2.peso = INT_MAX/2;
    coli2.x = final.x;
    coli2.y = final.y;

    //repetir los siguientes pasos hasta que se encuentre el camino mínimo posible
    while(coli1.peso + coli2.peso + promesa(coli1) + promesa(coli2) > mont_ini->vector[0].peso + mont_fin->vector[0].peso + promesa(mont_ini->vector[0]) + promesa(mont_fin->vector[0])){
        aux = sacar_cabeza(mont_ini);
        meter_vecinos(mont_ini, aux);
        aux = sacar_cabeza(mont_fin);
        meter_vecinos(mont_fin, aux);

        //condicione ver mientras se procesa
        if(mostrar_proceso && cont > iteraciones){
            cont = 0;
            if(delay)
                Sleep(delay);
        }
        cont++;
        cont2++;
    }

    //una vez terminado se cambia el array viene_de para que del inicio valla al final

    //Hacer que coli1 sea el que pertenece al grupo de inicio y coli2 sea el que pertenece al grupo final
    if(visitados[coli1.x][coli1.y] < 0){
        aux = coli1;
        coli1 = coli2;
        coli2 = aux;
    }

    char tipo;

    //ir cambiando la dirección hasta encontrar el origen
    while(viene_de[coli1.x][coli1.y]){
        tipo = viene_de[coli1.x][coli1.y];

        viene_de[coli1.x][coli1.y] = tipo_de_adyacencia(coli2.x - coli1.x, coli2.y - coli1.y);

        coli2 = coli1;
        coli1.x += inv_tipo_de_adyacencia(tipo).x;
        coli1.y += inv_tipo_de_adyacencia(tipo).y;
    }


    viene_de[coli1.x][coli1.y] = tipo_de_adyacencia(coli2.x - coli1.x, coli2.y - coli1.y);


    //ir cambiando el valor de visitados y el tamaño del camino encontrado
    tam = 1;
    while(viene_de[coli1.x][coli1.y]){
        tam++;
        visitados[coli1.x][coli1.y] = 3;
        aux = coli1;
        coli1.x += inv_tipo_de_adyacencia(viene_de[aux.x][aux.y]).x;
        coli1.y += inv_tipo_de_adyacencia(viene_de[aux.x][aux.y]).y;
    }
    visitados[coli1.x][coli1.y] = 3;
}


//Imprimir camino
void imprimir_camino(cords local){
    cords aux;
    printf("Camino: (%i, %i) ", local.x, local.y);
    while(viene_de[local.x][local.y]){
        aux = local;
        local.x += inv_tipo_de_adyacencia(viene_de[aux.x][aux.y]).x;
        local.y += inv_tipo_de_adyacencia(viene_de[aux.x][aux.y]).y;
        printf("(%i, %i) ", local.x, local.y);
    }
}

//cambia el fondo de pantalla basándonos en el valor de visitados en x y
void setBg(int x, int y){
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    switch (visitados[x][y]) {
        case 3:
            SetConsoleTextAttribute(hConsole, FOREGROUND_GREEN | FOREGROUND_BLUE | FOREGROUND_RED);
            break;
        case 2:
            SetConsoleTextAttribute(hConsole, FOREGROUND_GREEN | FOREGROUND_INTENSITY);
            break;
        case 1:
            SetConsoleTextAttribute(hConsole, FOREGROUND_GREEN | FOREGROUND_BLUE);
            break;
        case -2:
            SetConsoleTextAttribute(hConsole, FOREGROUND_RED | FOREGROUND_INTENSITY);
            break;
        case -1:
            SetConsoleTextAttribute(hConsole, FOREGROUND_RED | FOREGROUND_BLUE);
            break;
        default:
            SetConsoleTextAttribute(hConsole, FOREGROUND_INTENSITY);
    }
}


void imprimir_array_resultado(){

    cords par;
    char last_type = 0;

    for(par.x = 0; par.x < size_x; par.x++) {
        for (par.y = 0; par.y < size_y; par.y++) {
            //escribir numero
            if(last_type != visitados[par.x][par.y]) {
                //cambiar el fondo de pantalla, solo si el último tipo ha cambiado
                setBg(par.x, par.y);
                last_type = visitados[par.x][par.y];
            }
            printf("%02hhX", pesos[par.x][par.y]);

            if(caminos) {
                //escribir espacio o guion ─ = 196
                if (viene_de[par.x][par.y] == DERECHA)
                    //el camino va de izquierda a derecha
                    printf("%c", 196);
                else if (is_in_bounds(par.x, par.y + 1) && viene_de[par.x][par.y + 1] == IZQUIERDA) {
                    //el camino va de derecha a izquierda (es necesario cambiar el color de fondo)
                    if(last_type != visitados[par.x][par.y + 1]) {
                        //cambiar el fondo de pantalla, solo si el último tipo ha cambiado
                        setBg(par.x, par.y + 1);
                        last_type = visitados[par.x][par.y + 1];
                    }
                    printf("%c", 196);
                } else
                    printf(" ");
            }
        }

        if(size_x > 100) {
            //volver la consola a su estado habitual
            last_type = 3;
            SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_GREEN | FOREGROUND_BLUE | FOREGROUND_RED);
            printf("  - %i", par.x);
        }

        printf("\n");
        if(caminos) {
            //imprimir espacios y conexiones
            for (par.y = 0; par.y < size_y; par.y++) {
                //escribir espacio o palo │ = 179
                if (viene_de[par.x][par.y] == ARRIBA) {
                    //el camino va de arriba a abajo
                    if(last_type != visitados[par.x][par.y]) {
                        //cambiar el fondo de pantalla, solo si el último tipo ha cambiado
                        setBg(par.x, par.y);
                        last_type = visitados[par.x][par.y];
                    }
                    printf("%c", 179);
                } else if (is_in_bounds(par.x + 1, par.y) && viene_de[par.x + 1][par.y] == ABAJO) {
                    //el camino va de abajo a arriba
                    if(last_type != visitados[par.x + 1][par.y]) {
                        //cambiar el fondo de pantalla, solo si el último tipo ha cambiado
                        setBg(par.x + 1, par.y);
                        last_type = visitados[par.x + 1][par.y];
                    }
                    printf("%c", 179);
                } else
                    printf(" ");


                //escribir espacio
                printf("  ");

            }
            printf("\n");
        }
    }

    //volver la consola a su estado habitual
    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 0x7);
}


static BITMAPINFO frame_bitmap_info;
static HBITMAP frame_bitmap = 0;
static HDC frame_device_context = 0;

LRESULT CALLBACK WindowProcessMessage(HWND, UINT, WPARAM, LPARAM);



void mostrar_pantalla(HWND window_handle){
    InvalidateRect(window_handle, NULL, FALSE);
    UpdateWindow(window_handle);
}

int get_pixel_color(char ** vis,unsigned char ** pes,int x,int y){
    int pixel_color;

    switch (vis[x][y]) {
        case -1:
        case 1:
            pixel_color = 0xffff00;
            break;
        case -2:
            pixel_color = 0xff0000;
            break;
        case 2:
            pixel_color = 0x00ff00;
            break;
        case 0:
            pixel_color = 0x000000;
            break;
        default:
            pixel_color = 0xffffff;
    }

    pixel_color |= pes[x][y];

    return pixel_color;
}

int get_flecha_color(char dir,double x,double y, int pixel_color){
    //cambiar los casos para que sean como una flecha hacia abajo
    double aux;
    switch (dir) {
        case ARRIBAP:
            //flecha hacia arriba
            y = 1 - y;
            break;

        case ABAJOP:
            //flecha hacia abajo
            break;

        case IZQUIERDAP:
            //flecha hacia izquierda
            aux = x;
            x = 1 - y;
            y = aux;
            break;

        case DERECHAP:
            //flecha hacia derecha
            aux = y;
            y = 1 - x;
            x = aux;
            break;

        default:
            return 0x000000;
    }

    if(y > 0.5){
        if(x >= 0.3 && x <= 0.7)
            return pixel_color;//dibujar el palo
        else
            return 0x000000;
    }else{
        if(x < 0.5){
            if(y >= 0.5 - x)
                return pixel_color;//parte izquierda de la punta
            else
                return 0x000000;
        }else{
            if(y >= x - 0.5)
                return pixel_color;//parte derecha de la punta
            else
                return 0x000000;
        }

    }
}

void fill_pantalla(struct data * pantalla,unsigned char ** pes, char ** vis, char ** prov, double L, double B, double pixelsPorCelda, int sizx, int sizy){
    int x, y, Ax, Ay;
    int pixel_color;
    double Axd, Ayd;

    for (y = 0; y < pantalla->height; y++)
        for (x = 0; x < pantalla->width; x++) {

            Axd = L + (x / pixelsPorCelda);
            Ax = floor(Axd);
            Ayd = B + (y / pixelsPorCelda);
            Ay = floor(Ayd);

            if(Ax >= sizx || Ax < 0 || Ay >= sizy || Ay < 0){
                pantalla->pixels[y * pantalla->width + x] = 0x000000;
                continue;
            }


            if(pixelsPorCelda < 16) {
                //modo general

                pixel_color = get_pixel_color(vis, pes, Ax, Ay);

            }else{
                //modo detallado

                //suma y resta para que esté centrado
                Axd -= 0.25;
                Ax = floor(Axd);
                Ayd += 0.25;
                Ay = floor(Ayd);

                if(Ax >= sizx || Ax < 0 || Ay >= sizy || Ay < 0){
                    pantalla->pixels[y * pantalla->width + x] = 0x000000;
                    continue;
                }

                //get subpixel
                if(Axd - Ax < 0.5){
                    if(Ayd - Ay < 0.5){
                        //subpixel izquierda abajo (flecha hacia arriba o abajo)
                        pixel_color = 0x000000;

                        if(Ay > 0)
                            if(prov[Ax][Ay - 1] == ARRIBAP)
                                pixel_color |= get_flecha_color(ARRIBAP, (Axd - Ax) * 2, (Ayd - Ay) * 2, get_pixel_color(vis, pes,Ax, Ay - 1));

                        if(prov[Ax][Ay] == ABAJOP)
                            pixel_color |= get_flecha_color(ABAJOP, (Axd - Ax) * 2, (Ayd - Ay) * 2, get_pixel_color(vis, pes, Ax, Ay));

                    }else{
                        //subpixel izquierda arriba (caso genérico)
                        pixel_color = get_pixel_color(vis, pes, Ax, Ay);
                    }
                } else{
                    if(Ayd - Ay < 0.5){
                        //subpixel derecha abajo (siempre negro)
                        pixel_color = 0x000000;
                    }else{
                        //subpixel derecha arriba (flecha hacia derecha o izquierda)
                        pixel_color = 0x000000;

                        if(Ax < sizx - 1)
                            if(prov[Ax + 1][Ay] == IZQUIERDAP)
                                pixel_color |= get_flecha_color(IZQUIERDAP, (Axd - Ax - 0.5) * 2, (Ayd - Ay - 0.5) * 2, get_pixel_color(vis, pes, Ax + 1, Ay));

                        if(prov[Ax][Ay] == DERECHAP)
                            pixel_color |= get_flecha_color(DERECHAP, (Axd - Ax - 0.5) * 2, (Ayd - Ay - 0.5) * 2, get_pixel_color(vis, pes, Ax, Ay));
                    }
                }

            }

            pantalla->pixels[y * pantalla->width + x] = pixel_color;
        }

    //caso especial para que siempre se vea el camino final da igual la resolución
    if(pixelsPorCelda < 1 && visitados[inicio.x][inicio.y] == 3){
        Ax = inicio.x;
        Ay = inicio.y;

        int auxx, auxy;

        while(viene_de[Ax][Ay]) {
            Axd = (Ax - L) * pixelsPorCelda;
            Ayd = (Ay - B) * pixelsPorCelda;

            if(Axd >= 0 && Axd < pantalla->width && Ayd >= 0 && Ayd < pantalla->height)
                pantalla->pixels[(int)(floor(Ayd) * pantalla->width + floor(Axd))] = 0xffffff;

            auxx = inv_tipo_de_adyacencia(prov[Ax][Ay]).x;
            auxy = inv_tipo_de_adyacencia(prov[Ax][Ay]).y;

            Ax += auxx;
            Ay += auxy;
        }

    }

}


DWORD WINAPI threadPantalla(LPVOID param){

    thread_args * args = param;

    WNDCLASS window_class = {0};
    const char window_class_name[] = "My Window Class";
    window_class.lpszClassName = window_class_name;
    window_class.lpfnWndProc = WindowProcessMessage;
    window_class.hInstance = args->hInstance;

    RegisterClass(&window_class);

    frame_bitmap_info.bmiHeader.biSize = sizeof(frame_bitmap_info.bmiHeader);
    frame_bitmap_info.bmiHeader.biPlanes = 1;
    frame_bitmap_info.bmiHeader.biBitCount = 32;
    frame_bitmap_info.bmiHeader.biCompression = BI_RGB;
    frame_device_context = CreateCompatibleDC(0);

    RECT desktop_rect;
    HWND desktop_handle = GetDesktopWindow();
    if(desktop_handle) GetWindowRect(desktop_handle, &desktop_rect);
    else { desktop_rect.left = 0; desktop_rect.top = 0; desktop_rect.right = 800; desktop_rect.bottom = 600; }
    HWND window_handle = CreateWindow((PCSTR)window_class_name, "pantalla", WS_POPUP, desktop_rect.left,desktop_rect.top, desktop_rect.right - desktop_rect.left,desktop_rect.bottom - desktop_rect.top, NULL, NULL, args->hInstance, NULL);

    MSG message;
    ShowWindow(window_handle, args->nCmdShow);

    double T, L = 0, B = 0, R;

    L = 0;
    B = 0;

    double pixelsPorCelda;


    int last_mouse_x, last_mouse_y, has_last_mouse = 0, mxd, myd;

    if(args->sizez_x / (double)args->size_y < args->frame->width / (double)args->frame->height){
        pixelsPorCelda = args->frame->height / (double) args->size_y;
    }else{
        pixelsPorCelda = args->frame->width / (double) args->sizez_x;
    }

    while(!args->quit){//repetir mientras no se sale

        //procesar mensajes
        args->mouse->buttons &= ~(MOUSE_SCROLL_DOWN | MOUSE_SCROLL_UP);
        while(PeekMessage(&message, NULL, 0, 0, PM_REMOVE)) {
            TranslateMessage(&message);
            DispatchMessage(&message);
        }

        //hacer zoom o no
        if(args->mouse->buttons & (MOUSE_SCROLL_DOWN)){
            //quitar zoom
            pixelsPorCelda *= ZOOM;

            B -= args->mouse->y * (1 - ZOOM) / pixelsPorCelda;
            L -= args->mouse->x * (1 - ZOOM) / pixelsPorCelda;
        }

        if(args->mouse->buttons & (MOUSE_SCROLL_UP)){
            //aumentar zoom
            B += args->mouse->y * (1 - ZOOM) / pixelsPorCelda;
            L += args->mouse->x * (1 - ZOOM) / pixelsPorCelda;

            pixelsPorCelda /= ZOOM;
        }

        //get la distancia movida por el ratón
        if(args->mouse->buttons & (MOUSE_LEFT | MOUSE_RIGHT | MOUSE_MIDDLE)) {
            if(has_last_mouse){
                mxd = args->mouse->x - last_mouse_x;
                myd = args->mouse->y - last_mouse_y;
                last_mouse_x = args->mouse->x;
                last_mouse_y = args->mouse->y;
            }else{
                has_last_mouse = 1;
                last_mouse_x = args->mouse->x;
                last_mouse_y = args->mouse->y;
                mxd = 0;
                myd = 0;
            }
        }else{
            has_last_mouse = 0;
        }

        //mover distancias si se ha movido el ratón
        if(has_last_mouse){
            L -= mxd / pixelsPorCelda;
            B -= myd / pixelsPorCelda;
        }

        //rellenar pixeles
        fill_pantalla(args->frame, args->pesos, args->visitados, args->viene_de, L, B, pixelsPorCelda, args->sizez_x, args->size_y);

        //mostrar pixels por pantalla
        mostrar_pantalla(window_handle);
    }

    return 0;
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PSTR pCmdLine, int nCmdShow) {

    FILE* archivo = fopen("resultados.txt", "w");

    int val, cond, pesos_def, arr, camino;
    double ini, fin;
    //get tamaño
    printf("Introduce el tamano en x e y del array\n");
    scanf("%i %i", &size_x, &size_y);

    fprintf(archivo,"%i\n%i\n", size_x, size_y);



    srand(time(NULL));

    //rellenar array y asignar memoria a pesos
    pesos = (unsigned char **) ini_array(sizeof(unsigned char));
    printf("\nMemoria para el array de pesos asignada: ");
    unsigned long int tamano = size_x * size_y * sizeof(unsigned char);
    imprimir_tamano(tamano);
    printf("\n");

    array_fill();

    printf("\nMostrar array generado?");
    scanf("%i", &arr);

    if(arr){
        printf("Darle color?");
        scanf("%i", &color);
    }

    if(arr){
        printf("\n\n\n");
        print_arr_esp(pesos);
    }

    printf("\nMostrar arrays auxiliares?");
    scanf("%i", &cond);

    printf("\nMostrar el camino resultante?");
    scanf("%i", &camino);

    printf("\nMostrar el array resultante?");
    scanf("%i", &arr);

    if(arr) {
        printf("\nMostrar caminos al imprimir resultado?");
        scanf("%i", &caminos);
    }

    printf("\nMostrar mensajes de error al asignar memoria?");
    scanf("%i", &error);

    printf("\nQuieres ver el proceso?");
    scanf("%i", &mostrar_proceso);

    if(mostrar_proceso) {
        printf("Con cuanto delay por muestra (ms)?");
        scanf("%i", &delay);

        printf("Cada cuantas iteraciones?");
        scanf("%i", &iteraciones);
    }

    printf("\nUtilizar inicio y final custom?");
    scanf("%i", &pesos_def);
    if(!pesos_def){
        inicio.x = 0;
        inicio.y = 0;
        final.x = size_x-1;
        final.y = size_y-1;
    }else {
        printf("\nDonde quieres empezar y acabar?\n");
        do {
            scanf("%i %i %i %i", &inicio.x, &inicio.y, &final.x, &final.y);

            if (!(is_in_bounds(inicio.x, inicio.y) && is_in_bounds(final.x, final.y)))
                printf("Fuera de rango\n");

        } while (!(is_in_bounds(inicio.x, inicio.y) && is_in_bounds(final.x, final.y)));
    }

    printf("\n\n\nIniciando proceso...\n");

    //asignar memoria 2
    visitados = (char **) ini_array(sizeof(char));
    printf("\nMemoria para el array de visitados asignada: ");
    tamano = size_x * size_y * sizeof(char);
    imprimir_tamano(tamano);
    printf("\n");

    viene_de = (char **)ini_array(sizeof(char));
    printf("\nMemoria para el array de procedencia asignada: ");
    tamano = size_x * size_y * sizeof(char);
    imprimir_tamano(tamano);
    printf("\n");

    mont_ini = malloc(sizeof(struct monticulo_minimos));
    mont_fin = malloc(sizeof(struct monticulo_minimos));

    //teóricamente, el número de elementos en la fronterá máximos (si es que no tiene huecos) es de size_x + size_y, algunos caminos ocuparán más memoria que esto, por eso es necesario reasignar memoria a veces al meter elementos en el montículo
    mont_ini->vector = malloc((size_x + size_y) * sizeof(pes_cords) * 2);
    printf("\nMemoria para el array de monticulo inicial asignada: ");
    tamano = (size_x + size_y) * sizeof(pes_cords) * 2;
    imprimir_tamano(tamano);
    printf("\n");

    mont_fin->vector = malloc((size_x + size_y) * sizeof(pes_cords) * 2);
    printf("\nMemoria para el array de monticulo final asignada: ");
    tamano = (size_x + size_y) * sizeof(pes_cords) * 2;
    imprimir_tamano(tamano);
    printf("\n");

    mont_ini->vector_size = (size_x + size_y) * 2;
    mont_fin->vector_size = (size_x + size_y) * 2;


    //hacer nuevo thread con la pantalla

    fill_arrs();

    printf("\n\n\nArrays de setup rellenados, iniciando calculo del camino\n\n");


    LPDWORD thread = malloc(sizeof(DWORD));

    frame = malloc(sizeof(struct data));

    mouse = malloc(sizeof(struct mouse_dat));

    keyboard = malloc(sizeof(char)* 256);

    thread_args *args = malloc(sizeof(thread_args));

    args->pesos = pesos;
    args->size_y = size_y;
    args->sizez_x = size_x;
    args->quit = 0;
    quitflag = &args->quit;
    args->frame = frame;
    args->hInstance = hInstance;
    args->nCmdShow = nCmdShow;
    args->pesos = pesos;
    args->visitados = visitados;
    args->viene_de = viene_de;
    args->mouse = mouse;
    args->keyboard = keyboard;

    if(mostrar_proceso) {
        CreateThread(NULL, 0, threadPantalla, args, 0, thread);
    }

    ini = microsegundos();
    if(!(inicio.x == final.x && inicio.y == final.y))
        buscar_camino_de_esquina_a_esquina();
    else{
        //caso especial en el que el principio es igual que el final
        coli1.x = inicio.x;
        coli1.y = inicio.y;
        coli1.peso = pesos[inicio.x][inicio.y];
        coli2.peso = 0;
        coli2.x = final.x;
        coli2.y = final.y;
        visitados[inicio.x][inicio.y] = 3;
    }
    fin = microsegundos();

    printf("\n\n\n");

    if(cond) {
        printf("\n\n\n");
        printf("Visitados:\n");
        print_arr_esp((unsigned char **)visitados);
        printf("\n\n\n");
        printf("Procedencia:\n");
        print_arr_esp((unsigned char **)viene_de);
        printf("\n\n\n");
        printf("El mayor peso considerado es: %X\n", mayor);
    }

    printf("Encontrado en: %lf segundos\n", (fin-ini)/1000000);
    printf("Se necesitaron %i iteraciones\n", cont2);
    printf("Se han comprobado %i posiciones\n", comprobados);
    printf("Se han anadido %i posiciones\n", anadidos);
    printf("Peso total: %i\n", coli1.peso + coli2.peso);
    printf("Tamano del camino: %i\n", tam);

    fprintf(archivo, "\n\n%lf\n%i\n%i\n%i\n%i\n%i", (fin-ini)/1000000, cont2, comprobados, anadidos, coli1.peso + coli2.peso, tam);

    if(camino) {
        imprimir_camino(inicio);
        printf("\n\n\n");
    }
    if(arr)
        imprimir_array_resultado();




    printf("\n\n\n\n");
    printf("Proceso terminado con exito :D (Pulsa enter para salir)");
    while (getchar() != '\n');
    getchar();

    *quitflag = 1;
    WaitForSingleObject(thread, INFINITE);

    //liberar memoria
    free_array((char **) pesos);
    free_array(visitados);
    free_array(viene_de);
    free(mont_ini->vector);
    free(mont_fin->vector);
    fclose(archivo);
    free(keyboard);
    free(thread);
    free(mouse);
    free(frame);
    free(args);

    return 0;
}

LRESULT CALLBACK WindowProcessMessage(HWND window_handle, UINT message, WPARAM wParam, LPARAM lParam) {
    static int has_focus = 1;

    switch(message) {

        case WM_PAINT: {
            static PAINTSTRUCT paint;
            static HDC device_context;
            device_context = BeginPaint(window_handle, &paint);
            BitBlt(device_context,
                   paint.rcPaint.left, paint.rcPaint.top,
                   paint.rcPaint.right - paint.rcPaint.left, paint.rcPaint.bottom - paint.rcPaint.top,
                   frame_device_context,
                   paint.rcPaint.left, paint.rcPaint.top,
                   SRCCOPY);
            EndPaint(window_handle, &paint);
        } break;

        case WM_SIZE: {
            frame_bitmap_info.bmiHeader.biWidth  = LOWORD(lParam);
            frame_bitmap_info.bmiHeader.biHeight = HIWORD(lParam);

            if(frame_bitmap) DeleteObject(frame_bitmap);
            frame_bitmap = CreateDIBSection(NULL, &frame_bitmap_info, DIB_RGB_COLORS, (void**)&frame->pixels, 0, 0);
            SelectObject(frame_device_context, frame_bitmap);

            frame->width =  LOWORD(lParam);
            frame->height = HIWORD(lParam);
        } break;



        case WM_KILLFOCUS: {
            has_focus = 0;
            memset(keyboard, 0, 256 * sizeof(keyboard[0]));
            mouse->buttons = 0;
        } break;

        case WM_SETFOCUS: has_focus = 1; break;

        case WM_SYSKEYDOWN:
        case WM_SYSKEYUP:
        case WM_KEYDOWN:
        case WM_KEYUP: {
            if(has_focus) {
                static char key_is_down, key_was_down;
                key_is_down  = (char)((lParam & (1 << 31)) == 0);
                key_was_down = (char)((lParam & (1 << 30)) != 0);
                if(key_is_down != key_was_down) {
                    keyboard[(int)wParam] = key_is_down;
                    if(key_is_down) {
                        switch(wParam) {
                            case VK_ESCAPE: *quitflag = 1; break;
                        }
                    }
                }
            }
        } break;

        case WM_MOUSEMOVE: {
            mouse->x = LOWORD(lParam);
            mouse->y = frame->height - 1 - HIWORD(lParam);
        } break;

        case WM_LBUTTONDOWN: mouse->buttons |=  MOUSE_LEFT;   break;
        case WM_LBUTTONUP:   mouse->buttons &= ~MOUSE_LEFT;   break;
        case WM_MBUTTONDOWN: mouse->buttons |=  MOUSE_MIDDLE; break;
        case WM_MBUTTONUP:   mouse->buttons &= ~MOUSE_MIDDLE; break;
        case WM_RBUTTONDOWN: mouse->buttons |=  MOUSE_RIGHT;  break;
        case WM_RBUTTONUP:   mouse->buttons &= ~MOUSE_RIGHT;  break;

        case WM_XBUTTONDOWN: {
            if(GET_XBUTTON_WPARAM(wParam) == XBUTTON1) {
                mouse->buttons |= MOUSE_X1;
            } else { mouse->buttons |= MOUSE_X2; }
        } break;
        case WM_XBUTTONUP: {
            if(GET_XBUTTON_WPARAM(wParam) == XBUTTON1) {
                mouse->buttons &= ~MOUSE_X1;
            } else { mouse->buttons &= ~MOUSE_X2; }
        } break;

        case WM_MOUSEWHEEL: {
            if(wParam & 0b10000000000000000000000000000000){
                //down
                mouse->buttons |= MOUSE_SCROLL_DOWN;
                mouse->buttons &= ~MOUSE_SCROLL_UP;
            }else{
                //up
                mouse->buttons |= MOUSE_SCROLL_UP;
                mouse->buttons &= ~MOUSE_SCROLL_DOWN;
            }
        } break;

        default: return DefWindowProc(window_handle, message, wParam, lParam);
    }

    return 0;
}