//////////////////////////////////////////////////////////////////
//           INSTITUTO POLITÉCNICO DO CÁVADO E DO AVE           //
//      ENGENHARIA DE SISTEMAS INFORMÁTICOS (PL) 2015/2016      //
//                                                              //
//                      TRABALHO PRÁTICO 2                      //
//                     VISÃO POR COMPUTADOR                     //
//                                                              //
//                   A3893 - ANDRÉ FERNANDES                    //
//                   A11198 - ANDRÉ MARTINS                     //
//                   A11201 - ÂNGELO FERREIRA                   //
//                                                              //
//////////////////////////////////////////////////////////////////

#define _CRT_SECURE_NO_WARNINGS
#define PI 3.14159265358979323846

#include <stdio.h>
#include <stdlib.h>
#include "cxcore.h"
#include "cv.h"
#include "highgui.h"
#include "vc.h"
#include <math.h>


int main(int argc, char **argv) {
	// Vídeo
	char *videofile = "video-tp2.avi";
	CvCapture *captureOrig;
	CvCapture *capture;
	IplImage *frameOrig;
	IplImage *frame;

	struct {
		int width, height;
		int ntotalframes;
		int fps;
		int nframe;
	} video;

	// Texto
	CvFont font, fontbkg;
	double hScale = 0.6;
	double vScale = 0.6;
	int lineWidth = 1;
	char str[500] = { 0 };
	int margem;

	// Outros
	int key = 0;
	int off_on = 1; // inicia a fazer categorização
	int nblobs;
	int xr = 0, yr = 0;
	double rad = 0;
	double area = 0;
	double perimeter = 0;

	int count = 0;
	int nOrange = 0;
	int diametro = 0;
	int calibre;

	float min = 1000;
	float max = 0;

	int i;
	OVC *blobs;

	/* Leitura de vídeo de um ficheiro */
	captureOrig = cvCaptureFromFile(videofile);
	capture = cvCaptureFromFile(videofile);

	/* Verifica se foi possível abrir o ficheiro de vídeo */
	if (!captureOrig || !capture) {
		fprintf(stderr, "Erro ao abrir o ficheiro de vídeo!\n");
		return 1;
	}

	/* Número total de frames no vídeo */
	video.ntotalframes = (int)cvGetCaptureProperty(captureOrig, CV_CAP_PROP_FRAME_COUNT);

	/* Frame rate do vídeo */
	video.fps = (int)cvGetCaptureProperty(captureOrig, CV_CAP_PROP_FPS);

	/* Resolução do vídeo */
	video.width = (int)cvGetCaptureProperty(captureOrig, CV_CAP_PROP_FRAME_WIDTH);
	video.height = (int)cvGetCaptureProperty(captureOrig, CV_CAP_PROP_FRAME_HEIGHT);

	/* Cria uma janela para exibir o vídeo */
	cvNamedWindow("VC - TP2", CV_WINDOW_AUTOSIZE);
	// cvNamedWindow("VC - TP2 - work", CV_WINDOW_AUTOSIZE); // desligar para aparecer apenas video trabalhado

	/* Inicializa a fonte */
	cvInitFont(&font, CV_FONT_HERSHEY_SIMPLEX, hScale, vScale, 0, lineWidth, 0);
	cvInitFont(&fontbkg, CV_FONT_HERSHEY_SIMPLEX, hScale, vScale, 0, lineWidth + 1, 0);

	while (key != 'q') {

		// modo
		if (key == 'z') {
			if (off_on == 0) { off_on = 1; key = 0; }
			else { off_on = 0; key = 0; }
		}
		else if (key == 'p') // pausa
		{
			key = 'w'; while (key != 'p') { key = cvWaitKey(10); }
		}

		// Leitura de uma frame do vídeo
		frameOrig = cvQueryFrame(captureOrig);
		frame = cvQueryFrame(capture);

		// Verifica se conseguiu ler a frame
		if (!frameOrig) break;
		if (!frame) break;

		// Número da frame a processar
		video.nframe = (int)cvGetCaptureProperty(captureOrig, CV_CAP_PROP_POS_FRAMES);

		// frame a trabalhar
		// cvShowImage("VC - TP2 - work", frame); // desligar para aparecer apenas video trabalhado

		// se modo caracterização
		if (off_on == 1)
		{
			// limpar frame
			cvPyrMeanShiftFiltering(frameOrig, frame, 31, 51, 2, cvTermCriteria(CV_TERMCRIT_ITER, 6, 0.1));

			// frame original para IVC
			IVC *ivcFrameOrig = vc_image_new(frameOrig->width, frameOrig->height, frameOrig->nChannels, frameOrig->depth);

			ivcFrameOrig->data = frameOrig->imageData;
			ivcFrameOrig->bytesperline = frameOrig->width*frameOrig->nChannels;

			// frame para IVC			
			IVC *ivcFrame = vc_image_new(frame->width, frame->height, frame->nChannels, frame->depth);
			IVC *ivcFrameRe = vc_image_new(frame->width, frame->height, frame->nChannels, frame->depth);
			IVC *ivcFrame_gray = vc_image_new(frame->width, frame->height, 1, 255);
			IVC *ivcFrame_bin = vc_image_new(frame->width, frame->height, 1, 255);

			ivcFrame->data = frame->imageData;
			ivcFrame->bytesperline = frame->width*frame->nChannels;


			//
			// tratamento 
			//

			// filtragem por cor
			vc_bgr_to_hsv_filter(ivcFrame);

			// calcular valores minimos e maximos
			min = vc_min_max(ivcFrame, min, 0);
			max = vc_min_max(ivcFrame, max, 1);

			// filtragem por minimos e maximos
			vc_min_max_filter(ivcFrame, min, max);

			// passagem para cinza
			vc_img_to_gray(ivcFrame, ivcFrame_bin);

			// passagem para binario
			vc_gray_to_binary_global_mean(ivcFrame_bin);

			// dilate
			vc_binary_dilate(ivcFrame_bin, ivcFrame_gray, 2);

			// blobs
			blobs = vc_binary_blob_labelling(ivcFrame_gray, ivcFrame_bin, &nblobs);

			vc_binary_blob_info(ivcFrame_bin, blobs, nblobs);

			system("cls");
			if (blobs != NULL)
			{
				for (i = 0; i < nblobs; i++)
				{
					if (blobs[i].area > ((ivcFrame_bin->width * ivcFrame_bin->height) / 20))
					{
						// raio
						xr = (int)(blobs[i].xc - blobs[i].x);
						yr = (int)(blobs[i].yc - blobs[i].y);

						if (xr > yr) rad = xr; else rad = yr;

						// inicio e fim do blob
						if (((blobs[i].yc - rad) > 1) && ((blobs[i].yc + rad) < ivcFrame_bin->height))
						{
							// contar blobs
							if (blobs[i].yc - rad >= 4 && blobs[i].yc - rad <= 10.2)
							{
								nOrange++;
							}


							printf("\nNumber of labels: %d\n\n", nblobs);

							printf("-> Label %d\n", blobs[i].label);
							printf("-> Area %d\n", blobs[i].area);
							printf("-> Perimetro %d\n", blobs[i].perimeter);
							printf("-> Raio %.2f\n", rad);
							printf("-> XC %d\n", blobs[i].xc);
							printf("-> YC %d\n\n", blobs[i].yc);

							// calculos
							diametro = (int)((rad * 2) * 55) / 280;
							area = PI * sqrt(rad);
							perimeter = 2 * PI * rad;

							if (diametro >= 53 && diametro < 60) { calibre = 13; }
							if (diametro >= 56 && diametro < 63) { calibre = 12; }
							if (diametro >= 58 && diametro < 66) { calibre = 11; }
							if (diametro >= 60 && diametro < 68) { calibre = 10; }
							if (diametro >= 62 && diametro < 70) { calibre = 9; }
							if (diametro >= 64 && diametro < 73) { calibre = 8; }
							if (diametro >= 67 && diametro < 76) { calibre = 7; }
							if (diametro >= 70 && diametro < 80) { calibre = 6; }
							if (diametro >= 73 && diametro < 84) { calibre = 5; }
							if (diametro >= 77 && diametro < 88) { calibre = 4; }
							if (diametro >= 81 && diametro < 92) { calibre = 3; }
							if (diametro >= 84 && diametro < 96) { calibre = 2; }
							if (diametro >= 87 && diametro < 100) { calibre = 1; }
							if (diametro >= 100) { calibre = 0; }

							margem = blobs[i].yc + rad + 40;

							sprintf(str, "Calibre: %d (D: %d mm)", calibre, diametro);
							cvPutText(frameOrig, str, cvPoint(blobs[i].xc - 75, margem + 25), &fontbkg, cvScalar(0, 0, 0, 0));
							cvPutText(frameOrig, str, cvPoint(blobs[i].xc - 75, margem + 25), &font, cvScalar(255, 0, 0, 0));

							sprintf(str, "Area: %.2f mm2", area);
							cvPutText(frameOrig, str, cvPoint(blobs[i].xc - 53, margem + 60), &fontbkg, cvScalar(0, 0, 0, 0));
							cvPutText(frameOrig, str, cvPoint(blobs[i].xc - 53, margem + 60), &font, cvScalar(255, 0, 0, 0));

							sprintf(str, "Perimetro: %.2f mm", perimeter);
							cvPutText(frameOrig, str, cvPoint(blobs[i].xc - 104, margem), &fontbkg, cvScalar(0, 0, 0, 0));
							cvPutText(frameOrig, str, cvPoint(blobs[i].xc - 104, margem), &font, cvScalar(255, 0, 0, 0));

							// desenhar centro
							cvCircle(frameOrig, cvPoint(blobs[i].xc, blobs[i].yc), 4, cvScalar(0, 255, 0, 0), 5, 8, 0);
							// desenhar o circulo
							cvCircle(frameOrig, cvPoint(blobs[i].xc, blobs[i].yc), rad, cvScalar(0, 0, 255, 0), 6, 8, 0);
						}
					}
				}
			}
			//return blobs;

		}

		/* Exemplo de inserção texto na frame */
		sprintf(str, "RESOLUCAO: %dx%d", video.width, video.height);
		cvPutText(frameOrig, str, cvPoint(20, 25), &fontbkg, cvScalar(0, 0, 0, 0));
		cvPutText(frameOrig, str, cvPoint(20, 25), &font, cvScalar(255, 0, 0, 0));
		sprintf(str, "TOTAL DE FRAMES: %d", video.ntotalframes);
		cvPutText(frameOrig, str, cvPoint(20, 50), &fontbkg, cvScalar(0, 0, 0, 0));
		cvPutText(frameOrig, str, cvPoint(20, 50), &font, cvScalar(255, 0, 0, 0));
		sprintf(str, "FRAME RATE: %d", video.fps);
		cvPutText(frameOrig, str, cvPoint(20, 75), &fontbkg, cvScalar(0, 0, 0, 0));
		cvPutText(frameOrig, str, cvPoint(20, 75), &font, cvScalar(255, 0, 0, 0));
		sprintf(str, "N. FRAME: %d", video.nframe);
		cvPutText(frameOrig, str, cvPoint(20, 100), &fontbkg, cvScalar(0, 0, 0, 0));
		cvPutText(frameOrig, str, cvPoint(20, 100), &font, cvScalar(255, 0, 0, 0));

		sprintf(str, "N. LARANJAS: %d", nOrange);
		cvPutText(frameOrig, str, cvPoint(20, 150), &fontbkg, cvScalar(0, 0, 0, 0));
		cvPutText(frameOrig, str, cvPoint(20, 150), &font, cvScalar(255, 0, 0, 0));


		//Shortcut Text
		sprintf(str, "Shortcuts:", NULL);
		cvPutText(frameOrig, str, cvPoint(1000, 25), &fontbkg, cvScalar(0, 0, 0, 0));
		cvPutText(frameOrig, str, cvPoint(1000, 25), &font, cvScalar(255, 0, 0, 0));
		sprintf(str, "     Q - Quit", NULL);
		cvPutText(frameOrig, str, cvPoint(1000, 50), &fontbkg, cvScalar(0, 0, 0, 0));
		cvPutText(frameOrig, str, cvPoint(1000, 50), &font, cvScalar(255, 0, 0, 0));
		sprintf(str, "     Z - Stop Img Process", NULL);
		cvPutText(frameOrig, str, cvPoint(1000, 75), &fontbkg, cvScalar(0, 0, 0, 0));
		cvPutText(frameOrig, str, cvPoint(1000, 75), &font, cvScalar(255, 0, 0, 0));
		sprintf(str, "     P - Pause", NULL);
		cvPutText(frameOrig, str, cvPoint(1000, 100), &fontbkg, cvScalar(0, 0, 0, 0));
		cvPutText(frameOrig, str, cvPoint(1000, 100), &font, cvScalar(255, 0, 0, 0));

		/* Exibe a frame */
		cvShowImage("VC - TP2", frameOrig);

		/* Exibe a frame a trabalhar */
		// cvShowImage("VC - TP2 - work", frame); // desligar para aparecer apenas video trabalhado

		/* Sai da aplicação, se o utilizador premir a tecla 'q' */
		key = cvWaitKey(10);
	}

	/* Fecha a janela */
	cvDestroyWindow("VC - TP2");
	// cvDestroyWindow("VC - TP2 - work"); // desligar para aparecer apenas video trabalhado

	/* Fecha o ficheiro de vídeo */
	cvReleaseCapture(&capture);
	return 0;
}
