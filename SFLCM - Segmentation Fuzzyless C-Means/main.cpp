//
//  main.cpp
//  SFLCM - Segmentation Fuzzyless C-Means
//
//  Created by Miguel Garcia on 03/12/18.
//  Copyright © 2018 Miguel Garcia. All rights reserved.
//


#include "header.h"


int main(int argc, char **argv) {
    printf
    ("------------------------------------------------------------------------\n");
    if (argc != 11&&argc!=12&&argc!=13) {
        printf("USAGE: fcm <input file>\n");
        exit(1);
    }
    
    double error;
    
    char *imageName = (char *)malloc(sizeof(char)*20), *centroidPath = (char *)malloc(sizeof(char)*20), *imageOutPath = (char *)malloc(sizeof(char)*20), *resultsPath = (char *)malloc(sizeof(char)*20); // image name
    imageName = argv[1]; // first agument for image name
    clusterN = atoi(argv[2]);
    centroidPath = argv[3];
    imageOutPath = argv[4];
    
    mejoraFlag=atoi(argv[5]);
    mejoraIteracion=atoi(argv[6]);
    resultsPath= argv[7];
    
    fuzziness=atof(argv[8]);
    epsilon=atof(argv[9]);
    umbral = atof(argv[10]);


    image=readImage(imageName);
    imageClusterAssigned = Mat::zeros(image.rows, image.cols, CV_32S);
    imageFlags=Mat::zeros(image.rows, image.cols, CV_32S);

    //pixelsNumber=image.rows*image.cols;
    
    centroidsPrincipales=readFileCentroids(clusterN, centroidPath);
  //  centroidsFlags=Mat::zeros(clusterN, 1, CV_8S);

   // membershipMatrix=Mat::zeros(image.rows*image.cols, clusterN, CV_8S);
    
    if (argc==12||argc==13) {
        char *resultadosIteracionesPath = (char *)malloc(sizeof(char)*20);
        resultadosIteracionesPath = argv[11];
        
        FILE *resultadosIteraciones;
        
        if ((resultadosIteraciones = fopen(resultadosIteracionesPath, "r")) == NULL) {
            printf("Failed to open input file.");
            return -1;
        }
        
        int i,j;
        
         if (argc==13) {
             numResultadosSinMejora=atoi(argv[12]);
         }
         else{
             printf("Sin numero de resultados indicado, se usará 1\n");
         }
        
        iteraciones=(double *) malloc(numResultadosSinMejora*sizeof(double));
        
        for (i = 0; i < numResultadosSinMejora; i++) {
            for (j = 0; j < 3; j++) {
                fscanf(resultadosIteraciones, "%lf", &iteraciones[i]);
            }
        }
        fclose(resultadosIteraciones);
        
        double prom=0;
        for (i = 0; i < numResultadosSinMejora; i++) {
            prom+=iteraciones[i];
       //     printf("%d\n",(int)iteraciones[i]);
        }
        prom/=numResultadosSinMejora;
        prom*=0.6;
        mejoraIteracion=(int)prom;
      //  printf("Mejora iteracion\n%d\n", mejoraIteracion);
        
         //  return 0;
    }

    
    degree_of_membs=(double ***) malloc(image.rows*sizeof(double **));
    
    for(int i=0;i<image.rows;i++){
        degree_of_membs[i]=(double **)malloc(image.cols*sizeof(double *));
    }
    
    for(int i=0;i<image.rows;i++){
        for (int j=0; j<image.cols;j++) {
            degree_of_membs[i][j]=(double *)malloc(clusterN*sizeof(double));
        }
    }

    //Matriz de pertenencia inicial

    double new_uij;
    for (int j = 0; j < clusterN; j++) {
        for (int i = 0; i < image.rows; i++) {
            for (int l = 0; l < image.cols; l++) {
                new_uij = get_new_value(i, j,l);
                degree_of_membs[i][l][j] = new_uij;
            }
        }
    }
    
    
    //Inicia agrupamiento
    
    double max_diff;
    iteracion=0;
    
    
    struct timeval t_ini, t_fin;
    double secs, tiempo;
    /*******Inicia tiempo*******/
    gettimeofday(&t_ini, NULL);
    /***************************/
    
     do {
         asignar();
         calculate_centre_vectors();
         max_diff = update_degree_of_membership();

         /*
         printf("Iteracion: %d\n",iteracion);
         
         printf("Max diff: %lf\n",max_diff);
         printf("Epsilon: %lf\n",epsilon);
*/

         iteracion++;
     } while (fabs(max_diff)> epsilon);
    
    
    /*******Termina tiempo*******/
    gettimeofday(&t_fin, NULL);
    /****************************/
    secs = timeval_diff(&t_fin, &t_ini);
    tiempo = (secs * 1000.0); //Tiempo en milisegundos
    
    error= SSE(image, imageClusterAssigned, centroidsPrincipales, clusterN);
    
    
    Results = fopen(resultsPath,"a");
    fprintf(Results,"%.16g\t %f\t %d\n", tiempo, error, iteracion); //Imprime en el archivo [Tiempo Error Iteracion]
    fclose(Results);
    printf("Iteraciones: %d\n",iteracion);
    
    display(image, imageClusterAssigned, centroidsPrincipales, clusterN, imageOutPath);
    
    printf("Termino\n");
    
    return 0;
}




