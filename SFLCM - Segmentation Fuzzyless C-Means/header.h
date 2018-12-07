//
//  header.h
//  SFLCM - Segmentation Fuzzyless C-Means
//
//  Created by Miguel Garcia on 04/12/18.
//  Copyright © 2018 Miguel Garcia. All rights reserved.
//

#ifndef header_h
#define header_h



#include <iostream>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>

using namespace cv;

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>

/*Borrar*/
#include <time.h>
/*Borrar*/

int pixelsNumber;
int clusterN;
int mejoraFlag;
int mejoraIteracion;

FILE *Results;

double ***degree_of_membs;

int num_dimensions=3;
double epsilon;
double fuzziness;
double umbral=0.000000003;
double umbral2=0.00001;

int iteracion;

int uno=1;

Mat image, centroidsPrincipales,centroidsFlags,imageClusterAssigned,imageFlags,newCentroids, membershipMatrix;



//Funciones
Mat readFileCentroids(int clusterN,char *path);
Mat readImage(char *path);
double get_new_value(int i, int j, int l);
void asignar();
int calculate_centre_vectors();
void crearObjetoEstable(int i,int l);
void display(Mat image, Mat clusters, Mat centroids, int clusterN, char *path);
double SSE(Mat image, Mat imageClusterAssigned, Mat centroidsPrincipales, int clusterN);

/* Image loading */
Mat readImage(char *path){
    
    Mat image = imread( path, 1 );
    
    if( !image.data )
    {
        printf( " La imagen no tiene datos \n " );
        return image;
    }
    
    int renglones = image.rows;
    int columnas = image. cols;
    
    // printf("renglones: %d - columnas: %d\n", renglones, columnas);
    
    //recorrer toda la imagen
    //las coordenadas inician en 0,0
    //int ren,col;//es lo mismo que x,y
    //los colores de los pixeles tienen rojo-r verde-g y azul-b
    //los colores con los vectores de caracteristicas de los pixeles
    //las coordenasdas tambien se pueden utilizar para clasificar los pixeles
    unsigned char r,g,b;
    for(int ren = 0; ren < renglones; ren++)
    {
        for(int col = 0; col < columnas; col++)
        {
            //opencv proporciona el rgb en el orden de bgr por eso primero leemos el azul-b
            //Este codigo es para la lectura de los colores de los pixeles
            b=image.at<Vec3b>(ren, col)[0];
            g=image.at<Vec3b>(ren, col)[1];
            r=image.at<Vec3b>(ren, col)[2];
            //Imprimimos en pantalla
            /*printf("Coordenadas: x: %d, y: %d - ", ren, col);
             printf ("R: %d, G: %d, B: %d \n", r, g, b);*/
            
            //si se desea alterar el color de un pixel para cada canal se debe hacer lo siguiente
            //image.at<Vec3b>(ren, col)[0]=nuevo valor;
        }
    }
    return (image);
    
}

Mat readFileCentroids(int clusterN,char *path){
    static char mydirIn[150] = "";// centroid path
    strcat(mydirIn,path);
    
    FileStorage file(mydirIn, FileStorage::READ);
    
    Mat centroids;// = Mat::zeros(clusterN,1, CV_8UC(3));
    
    file["centroids"] >> centroids;
    
    int renglones = centroids.rows;
    int columnas = centroids.cols;
    // printf("renglones: %d - columnas: %d \n", renglones, columnas);
    
    unsigned char r,g,b;
    for(int i = 0; i < renglones; i++)
    {
        b=centroids.at<Vec3b>(i, 0)[0];
        g=centroids.at<Vec3b>(i, 0)[1];
        r=centroids.at<Vec3b>(i, 0)[2];
    }
   
    file.release();
    
    return centroids;
}


int calculate_centre_vectors() {
    int i, j, k;
    double numerator, denominator;
    double ***t;
    
    t=(double ***) malloc(image.rows*sizeof(double **));
    for(i=0;i<image.rows;i++){
        t[i]=(double **)malloc(image.cols*sizeof(double*));
    }
    
    for(i=0;i<image.rows;i++){
        for (j=0; j<image.cols; j++) {
            t[i][j]=(double *)malloc(clusterN*sizeof(double));
        }
    }
    
    for (i = 0; i < image.rows; i++) {
        for (j = 0; j < image.cols; j++) {
            for (k=0; k<clusterN; k++) {
                t[i][j][k] = pow(degree_of_membs[i][j][k], fuzziness);
            }
        }
    }
    
    double sumR,sumG,sumB;
    int cont;
    int l;
    for (l = 0; l < clusterN; l++) {
        sumR=0;
        sumG=0;
        sumB=0;
        cont=0;
        numerator = 0.0;
        denominator = 0.0;
        for (i = 0; i < image.rows; i++) {
            for (j=0; j<image.cols; j++) {
                if (imageClusterAssigned.at<int>(i,j)==l) {
                    cont++;
                    sumR+=image.at<Vec3b>(i,j)[0];
                    sumG+=image.at<Vec3b>(i,j)[1];
                    sumB+=image.at<Vec3b>(i,j)[2];
                }
            }
        }
        centroidsPrincipales.at<Vec3b>(l,0)[0]=int(sumR/cont);
        centroidsPrincipales.at<Vec3b>(l,0)[1]=int(sumG/cont);
        centroidsPrincipales.at<Vec3b>(l,0)[2]=int(sumB/cont);
    }
    return 0;
}

void asignar(){
    int i, j, k, cluster;
    double highest;
    for (i = 0; i < image.rows; i++) {
        for (j=0; j<image.cols&&imageFlags.at<int>(i,j)!=uno; j++) {
            cluster = 1e12;
            highest = 0.0;
            for (k = 0; k < clusterN; k++) {
                if (degree_of_membs[i][j][k] > highest) {
                    highest = degree_of_membs[i][j][k];
                    cluster = k;
                }
            }
            //         printf("Hola\n");
            imageClusterAssigned.at<int>(i, j)=cluster;
        }
    }
}

double
get_norm(int i, int j, int l) {
    int k;
    double sum = 0.0;
    for (k = 0; k < num_dimensions; k++) {
        sum+=pow(image.at<Vec3b>(i, l)[k] - centroidsPrincipales.at<Vec3b>(j, 0)[k],2);
    }
    return sqrt(sum);
}

double get_new_value(int i, int j, int l) {
    int k;
    double t, p, sum;
    sum = 0.0;
    p = 2.0 / (fuzziness - 1.0);
    for (k = 0; k < clusterN; k++) {
        t = get_norm(i, j, l) / get_norm(i, k, l);
        t = pow(t, p);
        sum += t;
    }
    return (double)(1.0 / sum);
}

double update_degree_of_membership() {
    int i, j, l;
    double new_uij=0.0;
    double max_diff = 0.0, diff=0.0;
    
    
    for (i = 0; i < image.rows; i++) {
        for (l = 0; l < image.cols&&imageFlags.at<int>(i,l)!=uno; l++) {
            for (j = 0; j < clusterN; j++) {
                new_uij = get_new_value(i, j,l);
                diff = fabs((double)new_uij - (double)degree_of_membs[i][l][j]);
                
                if (diff > max_diff)
                    max_diff = diff;
                
                double anterior=degree_of_membs[i][l][j];
                double porcentaje=anterior*umbral;
                double diferencia=new_uij-anterior;
                diferencia=pow(diferencia, 2);
                diferencia=sqrt(diferencia);
                if(mejoraFlag==uno&&iteracion>mejoraIteracion) {
                    
                    if (diferencia>porcentaje) {
                        degree_of_membs[i][l][j] = new_uij;
                    }
                    else{
                        if (j==imageClusterAssigned.at<int>(i,l)) {
                            //                  printf("J ES IGUAL\n");
                            degree_of_membs[i][l][j] = new_uij;
                            crearObjetoEstable(i, l);
                        }
                        else{
                            //               printf("J NOOO ES IGUAL\n");
                            degree_of_membs[i][l][j] = new_uij;
                        }
                    }
                }
                else{
                    degree_of_membs[i][l][j] = new_uij;
                }
            }
        }
    }
    return max_diff;
}

void crearObjetoEstable(int i,int l){
    imageFlags.at<int>(i,l)=1;
}


void display(Mat image, Mat clusters, Mat centroids, int clusterN, char *path){
    int i,k;
    
    for (i = 0; i < clusterN; i++){ //cluster number
        
        for (int ren = 0; ren < image.rows; ren++){ //rows
            for (int col =0 ; col < image.cols; col++){ //colums
                
                if(clusters.at<int>(ren,col) == i){
                    
                    for (k = 0; k < num_dimensions; k++){ //Numero de caracteristicas
                        image.at<Vec3b>(ren,col)[k]=centroids.at<Vec3b>(i,0)[k];
                    }
                }
            }
        }
    }
    imwrite( path, image );
}

/* Retorna "a - b" en segundos */
double timeval_diff(struct timeval *a, struct timeval *b){
    return
    (double)(a->tv_sec + (double)a->tv_usec/1000000) -
    (double)(b->tv_sec + (double)b->tv_usec/1000000);
}

double SSE(Mat image, Mat imageClusterAssigned, Mat centroidsPrincipales, int clusterN){
    int i,k;
    double error = 0;
    

        for (int ren = 0; ren < image.rows; ren++){ //Numero de centroides
            for (int col =0 ; col < image.cols; col++){
                int sum = 0;
                for (i = 0; i < clusterN; i++){ //Numero de objetos
                    for (k = 0; k < num_dimensions; k++){ //Numero de caracteristicas
                        sum+= pow( int(centroidsPrincipales.at<Vec3b>(i,0)[k]) - int(image.at<Vec3b>(ren,col)[k]) , 2);
                    }
                    sum*=degree_of_membs[ren][col][i];
            }
            error += sqrt(sum);
        }
    }
    return error;
    
}

#endif /* header_h */
