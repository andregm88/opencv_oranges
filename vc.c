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
#define MAX(a,b) (a>b?a:b)

#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include "vc.h"


//////////////////////////////////////////////////////////////////
//                           FUNÇÕES                            //
//////////////////////////////////////////////////////////////////


// alocar memória para uma imagem
IVC *vc_image_new(int width, int height, int channels, int levels)
{
	IVC *image = (IVC *)malloc(sizeof(IVC));

	if (image == NULL) return NULL;
	if ((levels <= 0) || (levels > 255)) return NULL;

	image->width = width;
	image->height = height;
	image->channels = channels;
	image->levels = levels;
	image->bytesperline = image->width * image->channels;
	image->data = (unsigned char *)malloc(image->width * image->height * image->channels * sizeof(char));

	if (image->data == NULL)
	{
		return vc_image_free(image);
	}

	return image;
}


// libertar memória de uma imagem
IVC *vc_image_free(IVC *image)
{
	if (image != NULL)
	{
		if (image->data != NULL)
		{
			free(image->data);
			image->data = NULL;
		}

		free(image);
		image = NULL;
	}

	return image;
}


char *netpbm_get_token(FILE *file, char *tok, int len)
{
	char *t;
	int c;

	for (;;)
	{
		while (isspace(c = getc(file)));
		if (c != '#') break;
		do c = getc(file);
		while ((c != '\n') && (c != EOF));
		if (c == EOF) break;
	}

	t = tok;

	if (c != EOF)
	{
		do
		{
			*t++ = c;
			c = getc(file);
		} while ((!isspace(c)) && (c != '#') && (c != EOF) && (t - tok < len - 1));

		if (c == '#') ungetc(c, file);
	}

	*t = 0;

	return tok;
}


long int unsigned_char_to_bit(unsigned char *datauchar, unsigned char *databit, int width, int height)
{
	int x, y;
	int countbits;
	long int pos, counttotalbytes;
	unsigned char *p = databit;

	*p = 0;
	countbits = 1;
	counttotalbytes = 0;

	for (y = 0; y < height; y++)
	{
		for (x = 0; x < width; x++)
		{
			pos = width * y + x;

			if (countbits <= 8)
			{
				// Numa imagem PBM:
				// 1 = Preto
				// 0 = Branco
				//*p |= (datauchar[pos] != 0) << (8 - countbits);

				// Na nossa imagem:
				// 1 = Branco
				// 0 = Preto
				*p |= (datauchar[pos] == 0) << (8 - countbits);

				countbits++;
			}
			if ((countbits > 8) || (x == width - 1))
			{
				p++;
				*p = 0;
				countbits = 1;
				counttotalbytes++;
			}
		}
	}

	return counttotalbytes;
}


void bit_to_unsigned_char(unsigned char *databit, unsigned char *datauchar, int width, int height)
{
	int x, y;
	int countbits;
	long int pos;
	unsigned char *p = databit;

	countbits = 1;

	for (y = 0; y < height; y++)
	{
		for (x = 0; x < width; x++)
		{
			pos = width * y + x;

			if (countbits <= 8)
			{
				// Numa imagem PBM:
				// 1 = Preto
				// 0 = Branco
				//datauchar[pos] = (*p & (1 << (8 - countbits))) ? 1 : 0;

				// Na nossa imagem:
				// 1 = Branco
				// 0 = Preto
				datauchar[pos] = (*p & (1 << (8 - countbits))) ? 0 : 1;

				countbits++;
			}
			if ((countbits > 8) || (x == width - 1))
			{
				p++;
				countbits = 1;
			}
		}
	}
}


IVC *vc_read_image(char *filename)
{
	FILE *file = NULL;
	IVC *image = NULL;
	unsigned char *tmp;
	char tok[20];
	long int size, sizeofbinarydata;
	int width, height, channels;
	int levels = 255;
	int v;

	// Abre o ficheiro
	if ((file = fopen(filename, "rb")) != NULL)
	{
		// Efectua a leitura do header
		netpbm_get_token(file, tok, sizeof(tok));

		if (strcmp(tok, "P4") == 0) { channels = 1; levels = 1; }	// Se PBM (Binary [0,1])
		else if (strcmp(tok, "P5") == 0) channels = 1;				// Se PGM (Gray [0,MAX(level,255)])
		else if (strcmp(tok, "P6") == 0) channels = 3;				// Se PPM (RGB [0,MAX(level,255)])
		else
		{
#ifdef VC_DEBUG
			printf("ERROR -> vc_read_image():\n\tFile is not a valid PBM, PGM or PPM file.\n\tBad magic number!\n");
#endif

			fclose(file);
			return NULL;
		}

		if (levels == 1) // PBM
		{
			if (sscanf(netpbm_get_token(file, tok, sizeof(tok)), "%d", &width) != 1 ||
				sscanf(netpbm_get_token(file, tok, sizeof(tok)), "%d", &height) != 1)
			{
#ifdef VC_DEBUG
				printf("ERROR -> vc_read_image():\n\tFile is not a valid PBM file.\n\tBad size!\n");
#endif

				fclose(file);
				return NULL;
			}

			// Aloca mem�ria para imagem
			image = vc_image_new(width, height, channels, levels);
			if (image == NULL) return NULL;

			sizeofbinarydata = (image->width / 8 + ((image->width % 8) ? 1 : 0)) * image->height;
			tmp = (unsigned char *)malloc(sizeofbinarydata);
			if (tmp == NULL) return 0;

#ifdef VC_DEBUG
			printf("\nchannels=%d w=%d h=%d levels=%d\n", image->channels, image->width, image->height, levels);
#endif

			if ((v = fread(tmp, sizeof(unsigned char), sizeofbinarydata, file)) != sizeofbinarydata)
			{
#ifdef VC_DEBUG
				printf("ERROR -> vc_read_image():\n\tPremature EOF on file.\n");
#endif

				vc_image_free(image);
				fclose(file);
				free(tmp);
				return NULL;
			}

			bit_to_unsigned_char(tmp, image->data, image->width, image->height);

			free(tmp);
		}
		else // PGM ou PPM
		{
			if (sscanf(netpbm_get_token(file, tok, sizeof(tok)), "%d", &width) != 1 ||
				sscanf(netpbm_get_token(file, tok, sizeof(tok)), "%d", &height) != 1 ||
				sscanf(netpbm_get_token(file, tok, sizeof(tok)), "%d", &levels) != 1 || levels <= 0 || levels > 255)
			{
#ifdef VC_DEBUG
				printf("ERROR -> vc_read_image():\n\tFile is not a valid PGM or PPM file.\n\tBad size!\n");
#endif

				fclose(file);
				return NULL;
			}

			// Aloca mem�ria para imagem
			image = vc_image_new(width, height, channels, levels);
			if (image == NULL) return NULL;

#ifdef VC_DEBUG
			printf("\nchannels=%d w=%d h=%d levels=%d\n", image->channels, image->width, image->height, levels);
#endif

			size = image->width * image->height * image->channels;

			if ((v = fread(image->data, sizeof(unsigned char), size, file)) != size)
			{
#ifdef VC_DEBUG
				printf("ERROR -> vc_read_image():\n\tPremature EOF on file.\n");
#endif

				vc_image_free(image);
				fclose(file);
				return NULL;
			}
		}

		fclose(file);
	}
	else
	{
#ifdef VC_DEBUG
		printf("ERROR -> vc_read_image():\n\tFile not found.\n");
#endif
	}

	return image;
}


int vc_write_image(char *filename, IVC *image)
{
	FILE *file = NULL;
	unsigned char *tmp;
	long int totalbytes, sizeofbinarydata;

	if (image == NULL) return 0;

	if ((file = fopen(filename, "wb")) != NULL)
	{
		if (image->levels == 1)
		{
			sizeofbinarydata = (image->width / 8 + ((image->width % 8) ? 1 : 0)) * image->height + 1;
			tmp = (unsigned char *)malloc(sizeofbinarydata);
			if (tmp == NULL) return 0;

			fprintf(file, "%s %d %d\n", "P4", image->width, image->height);

			totalbytes = unsigned_char_to_bit(image->data, tmp, image->width, image->height);
			printf("Total = %ld\n", totalbytes);
			if (fwrite(tmp, sizeof(unsigned char), totalbytes, file) != totalbytes)
			{
#ifdef VC_DEBUG
				fprintf(stderr, "ERROR -> vc_read_image():\n\tError writing PBM, PGM or PPM file.\n");
#endif

				fclose(file);
				free(tmp);
				return 0;
			}

			free(tmp);
		}
		else
		{
			fprintf(file, "%s %d %d 255\n", (image->channels == 1) ? "P5" : "P6", image->width, image->height);

			if (fwrite(image->data, image->bytesperline, image->height, file) != image->height)
			{
#ifdef VC_DEBUG
				fprintf(stderr, "ERROR -> vc_read_image():\n\tError writing PBM, PGM or PPM file.\n");
#endif

				fclose(file);
				return 0;
			}
		}

		fclose(file);

		return 1;
	}

	return 0;
}


// passa por cor
int vc_bgr_to_hsv_filter(IVC *srcdst)
{
	unsigned char *data = (unsigned char *)srcdst->data;
	int width = srcdst->width;
	int height = srcdst->height;
	int bytesperline = srcdst->bytesperline;
	int channels = srcdst->channels;

	float r, g, b, hue, saturation, value;
	float rgb_max, rgb_min;
	int i, size;

	// Verificação de erros
	if ((srcdst->width <= 0) || (srcdst->height <= 0) || (srcdst->data == NULL)) return 0;
	if (channels != 3) return 0;

	size = width * height * channels;

	for (i = 0; i < size; i = i + channels)
	{
		b = (float)data[i];
		g = (float)data[i + 1];
		r = (float)data[i + 2];

		// Calcula valores maximo e minimo dos canais de cor R, G e B
		rgb_max = (r > g ? (r > b ? r : b) : (g > b ? g : b));
		rgb_min = (r < g ? (r < b ? r : b) : (g < b ? g : b));

		// Value toma valores entre [0,255]
		value = rgb_max;
		if (value == 0.0)
		{
			hue = 0.0;
			saturation = 0.0;
		}
		else
		{
			// Saturation toma valores entre [0,255]
			saturation = ((rgb_max - rgb_min) / rgb_max) * (float) 255.0;

			if (saturation == 0.0)
			{
				hue = 0.0;
			}
			else
			{
				// Hue toma valores entre [0,360]
				if ((rgb_max == r) && (g >= b))
				{
					hue = 60.0f * (g - b) / (rgb_max - rgb_min);
				}
				else if ((rgb_max == r) && (b > g))
				{
					hue = 360.0f + 60.0f * (g - b) / (rgb_max - rgb_min);
				}
				else if (rgb_max == g)
				{
					hue = 120 + 60 * (b - r) / (rgb_max - rgb_min);
				}
				else /* rgb_max == b*/
				{
					hue = 240.0f + 60.0f * (r - g) / (rgb_max - rgb_min);
				}
			}
		}

		// filtrar laranjas
		if (((hue >= 5) && (hue <= 35)) && (saturation / 255.0*100.0 >= 20) && (value / 255.0*100.0 >= 20))
		{
			data[i] = hue; // branco 
			data[i + 1] = saturation; // branco
			data[i + 2] = value; // branco
		}
		else
		{
			data[i] = 0; // preto
			data[i + 1] = 0; // preto
			data[i + 2] = 0; // preto
		}
	}
	return 1;
}

// calcular minimo ou maximo
float vc_min_max(IVC *srcdst, float min_max, int mode)
{
	unsigned char *data = (unsigned char *)srcdst->data;
	int width = srcdst->width;
	int height = srcdst->height;
	int bytesperline = srcdst->bytesperline;
	int channels = srcdst->channels;

	float r, g, b, hue, saturation, value;
	float rgb_max, rgb_min;
	int i, size;

	// Verificação de erros
	if ((srcdst->width <= 0) || (srcdst->height <= 0) || (srcdst->data == NULL)) return 0;
	if (channels != 3) return 0;

	size = width * height * channels;

	for (i = 0; i < size; i = i + channels)
	{
		hue = (float)data[i];
		saturation = (float)data[i + 1];
		value = (float)data[i + 2];

		// se modo = 0
		// calcula minimo
		if (mode == 0)
		{
			// filtrar azul e amarelo
			if (saturation < min_max && value != 0)
			{
				min_max = saturation;
			}
		}
		// se modo = 1
		// calcula maximo
		else if (mode == 1)
		{
			// filtrar azul e amarelo
			if (saturation > min_max)
			{
				min_max = saturation;
			}
		}
		else
		{
			return 0;
		}
	}
	return min_max;
}

// filtrar pelo min, media e max
int vc_min_max_filter(IVC *srcdst, float min, float max)
{
	unsigned char *data = (unsigned char *)srcdst->data;
	int width = srcdst->width;
	int height = srcdst->height;
	int bytesperline = srcdst->bytesperline;
	int channels = srcdst->channels;

	float r, g, b, hue, saturation, value;
	float rgb_max, rgb_min;
	int i, size;

	// Verificação de erros
	if ((srcdst->width <= 0) || (srcdst->height <= 0) || (srcdst->data == NULL)) return 0;
	if (channels != 3) return 0;

	size = width * height * channels;

	//float med = max - min;

	for (i = 0; i < size; i = i + channels)
	{
		hue = (float)data[i];
		saturation = (float)data[i + 1];
		value = (float)data[i + 2];

		/*if (saturation == min && saturation < (max - min))
		{
			data[i] = 255;
			data[i + 1] = 0;
			data[i + 2] = 0;
		}
		else*/ if (saturation >= (max - min))
		{
			data[i] = 0;
			data[i + 1] = 255;
			data[i + 2] = 0;
		}
		else
		{
			data[i] = 0;
			data[i + 1] = 0;
			data[i + 2] = 0;
		}
	}
	return 1;
}

// passar para cinza
int vc_img_to_gray(IVC *src, IVC *dst)
{
	unsigned char *datasrc = (unsigned char *)src->data;
	int bytesperline_src = src->width * src->channels;
	int channels_src = src->channels;

	unsigned char *datadst = (unsigned char *)dst->data;
	int bytesperline_dst = dst->width * dst->channels;
	int channels_dst = dst->channels;

	int width = src->width;
	int height = src->height;
	int x, y;
	long int pos_src, pos_dst;
	float rf, gf, bf;

	// verificação de erros
	if ((src->width <= 0) || (src->height <= 0) || (src->data == NULL)) return 0;
	if ((src->width != dst->width) || (src->height != dst->height)) return 0;
	if ((src->channels != 3) || (dst->channels != 1)) return 0;

	for (y = 0; y < height; y++)
	{
		for (x = 0; x < width; x++)
		{
			pos_src = y * bytesperline_src + x * channels_src;
			pos_dst = y * bytesperline_dst + x * channels_dst;

			rf = (float)datasrc[pos_src];
			gf = (float)datasrc[pos_src + 1];
			bf = (float)datasrc[pos_src + 2];

			if (rf == 255)
			{
				datadst[pos_dst] = 150;
			}
			else if (gf == 255)
			{
				datadst[pos_dst] = 255;
			}
			else
			{
				datadst[pos_dst] = 0;
			}
		}
	}
	return 1;
}

// passar para binario
int vc_gray_to_binary_global_mean(IVC *srcdst)
{
	unsigned char *data = (unsigned char *)srcdst->data;
	int width = srcdst->width;
	int height = srcdst->height;
	int bytesperline = srcdst->width * srcdst->channels;
	int channels = srcdst->channels;
	int x, y, soma = 0, tr;
	long int pos;

	//verificação de erros
	if ((srcdst->width <= 0) || (srcdst->height <= 0) || (srcdst->data == NULL)) return 0;
	if (channels != 1) return 0;

	// inverter imagem rgb
	for (y = 0; y < height; y++)
	{
		for (x = 0; x < width; x++)
		{
			pos = y * bytesperline + x * channels;

			soma += data[pos];
		}
	}

	tr = soma / (height*width);

	y = 0; x = 0;

	for (y = 0; y < height; y++)
	{
		for (x = 0; x < width; x++)
		{
			pos = y * bytesperline + x * channels;

			if (data[pos] > tr)
			{
				data[pos] = 255;
			}
			else
			{
				data[pos] = 0;
			}
		}
	}
	return 1;
}

// passar binario (apenas 255) para imagem
int vc_pix_to_frame(IVC *src, IVC *dst)
{
	unsigned char *datasrc = (unsigned char *)src->data;
	int bytesperline_src = src->width * src->channels;
	int channels_src = src->channels;

	unsigned char *datadst = (unsigned char *)dst->data;
	int bytesperline_dst = dst->width * dst->channels;
	int channels_dst = dst->channels;

	int width = src->width;
	int height = src->height;
	int x, y;
	long int pos_src, pos_dst;
	float rf, gf, bf;

	// verificação de erros
	if ((src->width <= 0) || (src->height <= 0) || (src->data == NULL)) return 0;
	if ((src->width != dst->width) || (src->height != dst->height)) return 0;
	if ((src->channels != 1) || (dst->channels != 3)) return 0;


	for (y = 0; y < height; y++)
	{
		for (x = 0; x < width; x++)
		{
			pos_src = y * bytesperline_src + x * channels_src;
			pos_dst = y * bytesperline_dst + x * channels_dst;

			if ((float)datasrc[pos_src] == 255)
			{
				datadst[pos_dst] = 255;
				datadst[pos_dst + 1] = 255;
				datadst[pos_dst + 2] = 255;

				//printf("%d / %d \n", pos_src, pos_dst);
			}/*
			else
			{
				datadst[pos_dst] = 255;
				datadst[pos_dst + 1] = 100;
				datadst[pos_dst + 2] = 100;
				//printf("%f / %f / %f --- ", datadst[pos_dst], datadst[pos_dst + 1], datadst[pos_dst + 2]);
			}*/
		}
	}
	return 1;
}

// trocar rgb <> bgr
int vc_change_rgb(IVC *srcdst)
{
	unsigned char *data = (unsigned char *)srcdst->data;
	int width = srcdst->width;
	int height = srcdst->height;
	int bytesperline = srcdst->bytesperline;
	int channels = srcdst->channels;

	float r, g, b, hue, saturation, value;
	float rgb_max, rgb_min;
	int i, size;

	// Verificação de erros
	if ((srcdst->width <= 0) || (srcdst->height <= 0) || (srcdst->data == NULL)) return 0;
	if (channels != 3) return 0;

	size = width * height * channels;

	for (i = 0; i < size; i = i + channels)
	{
		b = (float)data[i];
		g = (float)data[i + 1];
		r = (float)data[i + 2];


		data[i] = r;
		data[i + 1] = g;
		data[i + 2] = b;
	}
	return 1;
}

// Etiquetagem de blobs
// src		: Imagem binaria
// dst		: Imagem grayscale (ira conter as etiquetas)
// nlabels	: Endereço de memeria de uma variável inteira. Recebe o número de etiquetas encontradas.
// OVC*		: Retorna lista de estruturas de blobs (objectos), com respectivas etiquetas. � necess�rio libertar posteriormente esta mem�ria.
OVC* vc_binary_blob_labelling(IVC *src, IVC *dst, int *nlabels)
{
	unsigned char *datasrc = (unsigned char *)src->data;
	unsigned char *datadst = (unsigned char *)dst->data;
	int width = src->width;
	int height = src->height;
	int bytesperline = src->bytesperline;
	int channels = src->channels;
	int x, y, a, b;
	long int i, size;
	long int posX, posA, posB, posC, posD;
	int labeltable[256] = { 0 };
	int labelarea[256] = { 0 };
	int label = 1; // Etiqueta inicial.
	int num;
	OVC *blobs; // Apontador para lista de blobs (objectos) que ser� retornada desta fun��o.

	// Verificacao de erros
	if ((src->width <= 0) || (src->height <= 0) || (src->data == NULL)) return 0;
	if ((src->width != dst->width) || (src->height != dst->height) || (src->channels != dst->channels)) return NULL;
	if (channels != 1) return NULL;

	// Copia dados da imagem binaria para imagem grayscale
	memcpy(datadst, datasrc, bytesperline * height);

	// Serao atribuidas etiquetas no intervalo [1,254]
	// Este algoritmo esta assim limitado a 254 labels
	for (i = 0, size = bytesperline * height; i < size; i++)
	{
		if (datadst[i] != 0) datadst[i] = 255;
	}

	// Limpa os rebordos da imagem binaria
	for (y = 0; y < height; y++)
	{
		datadst[y * bytesperline + 0 * channels] = 0;
		datadst[y * bytesperline + (width - 1) * channels] = 0;
	}
	for (x = 0; x < width; x++)
	{
		datadst[0 * bytesperline + x * channels] = 0;
		datadst[(height - 1) * bytesperline + x * channels] = 0;
	}

	// Efectua a etiquetagem
	for (y = 1; y < height - 1; y++)
	{
		for (x = 1; x < width - 1; x++)
		{
			// Kernel:
			// A B C
			// D X

			posA = (y - 1) * bytesperline + (x - 1) * channels; // A
			posB = (y - 1) * bytesperline + x * channels; // B
			posC = (y - 1) * bytesperline + (x + 1) * channels; // C
			posD = y * bytesperline + (x - 1) * channels; // D
			posX = y * bytesperline + x * channels; // X

													// Se o pixel foi marcado
			if (datadst[posX] != 0)
			{
				if ((datadst[posA] == 0) && (datadst[posB] == 0) && (datadst[posC] == 0) && (datadst[posD] == 0))
				{
					datadst[posX] = label;
					labeltable[label] = label;
					label++;
				}
				else
				{
					num = 255;

					// Se A esta marcado, ja tem etiqueta (ja nao � 255), e � menor que a etiqueta "num"
					if ((datadst[posA] != 0) && (datadst[posA] != 255) && (datadst[posA] < num))
					{
						num = datadst[posA];
					}
					// Se B est� marcado, j� tem etiqueta (j� n�o � 255), e � menor que a etiqueta "num"
					if ((datadst[posB] != 0) && (datadst[posB] != 255) && (datadst[posB] < num))
					{
						num = datadst[posB];
					}
					// Se C est� marcado, j� tem etiqueta (j� n�o � 255), e � menor que a etiqueta "num"
					if ((datadst[posC] != 0) && (datadst[posC] != 255) && (datadst[posC] < num))
					{
						num = datadst[posC];
					}
					// Se D est� marcado, j� tem etiqueta (j� n�o � 255), e � menor que a etiqueta "num"
					if ((datadst[posD] != 0) && (datadst[posD] != 255) && (datadst[posD] < num))
					{
						num = datadst[posD];
					}

					// Actualiza a tabela de etiquetas
					if ((datadst[posA] != 0) && (datadst[posA] != 255))
					{
						if (labeltable[datadst[posA]] != labeltable[num])
						{
							for (a = 1; a < label; a++)
							{
								if (labeltable[a] == labeltable[datadst[posA]])
								{
									labeltable[a] = labeltable[num];
								}
							}
						}
					}
					if ((datadst[posB] != 0) && (datadst[posB] != 255))
					{
						if (labeltable[datadst[posB]] != labeltable[num])
						{
							for (a = 1; a < label; a++)
							{
								if (labeltable[a] == labeltable[datadst[posB]])
								{
									labeltable[a] = labeltable[num];
								}
							}
						}
					}
					if ((datadst[posC] != 0) && (datadst[posC] != 255))
					{
						if (labeltable[datadst[posC]] != labeltable[num])
						{
							for (a = 1; a < label; a++)
							{
								if (labeltable[a] == labeltable[datadst[posC]])
								{
									labeltable[a] = labeltable[num];
								}
							}
						}
					}
					if ((datadst[posD] != 0) && (datadst[posD] != 255))
					{
						if (labeltable[datadst[posD]] != labeltable[num])
						{
							for (a = 1; a < label; a++)
							{
								if (labeltable[a] == labeltable[datadst[posD]])
								{
									labeltable[a] = labeltable[num];
								}
							}
						}
					}
					labeltable[datadst[posX]] = num;

					// Atribui a etiqueta ao pixel
					datadst[posX] = num;
				}
			}
		}
	}

	// Volta a etiquetar a imagem
	for (y = 1; y < height - 1; y++)
	{
		for (x = 1; x < width - 1; x++)
		{
			posX = y * bytesperline + x * channels; // X

			if (datadst[posX] != 0)
			{
				datadst[posX] = labeltable[datadst[posX]];
			}
		}
	}

	// Contagem do numero de blobs
	// Passo 1: Eliminar, da tabela, etiquetas repetidas
	for (a = 1; a < label - 1; a++)
	{
		for (b = a + 1; b < label; b++)
		{
			if (labeltable[a] == labeltable[b]) labeltable[b] = 0;
		}
	}
	// Passo 2: Conta etiquetas e organiza a tabela de etiquetas, para que nao hajam valores vazios (zero) entre etiquetas
	*nlabels = 0;
	for (a = 1; a < label; a++)
	{
		if (labeltable[a] != 0)
		{
			labeltable[*nlabels] = labeltable[a]; // Organiza tabela de etiquetas
			(*nlabels)++; // Conta etiquetas
		}
	}

	// Se nao ha blobs
	if (*nlabels == 0) return NULL;

	// Cria lista de blobs (objectos) e preenche a etiqueta
	blobs = (OVC *)calloc((*nlabels), sizeof(OVC));
	if (blobs != NULL)
	{
		for (a = 0; a < (*nlabels); a++) blobs[a].label = labeltable[a];
	}
	else return NULL;

	return blobs;
}

// info blobs
int vc_binary_blob_info(IVC *src, OVC *blobs, int nblobs)
{
	unsigned char *data = (unsigned char *)src->data;
	int width = src->width;
	int height = src->height;
	int bytesperline = src->bytesperline;
	int channels = src->channels;
	int x, y, i;
	long int pos;
	int xmin, ymin, xmax, ymax;
	long int sumx, sumy;

	// Verificação de erros
	if ((src->width <= 0) || (src->height <= 0) || (src->data == NULL)) return 0;
	if (channels != 1) return 0;

	// Conta área de cada blob
	for (i = 0; i < nblobs; i++)
	{
		xmin = width - 1;
		ymin = height - 1;
		xmax = 0;
		ymax = 0;

		sumx = 0;
		sumy = 0;

		blobs[i].area = 0;

		for (y = 1; y < height - 1; y++)
		{
			for (x = 1; x < width - 1; x++)
			{
				pos = y * bytesperline + x * channels;

				if (data[pos] == blobs[i].label)
				{
					// Área
					blobs[i].area++;

					// Centro de Gravidade
					sumx += x;
					sumy += y;

					// Bounding Box
					if (xmin > x) xmin = x;
					if (ymin > y) ymin = y;
					if (xmax < x) xmax = x;
					if (ymax < y) ymax = y;

					// Perímetro
					// Se pelo menos um dos quatro vizinhos não pertence ao mesmo label, então é um pixel de contorno
					if ((data[pos - 1] != blobs[i].label) || (data[pos + 1] != blobs[i].label) || (data[pos - bytesperline] != blobs[i].label) || (data[pos + bytesperline] != blobs[i].label))
					{
						blobs[i].perimeter++;
					}
				}
			}
		}

		// Bounding Box
		blobs[i].x = xmin;
		blobs[i].y = ymin;
		blobs[i].width = (xmax - xmin) + 1;
		blobs[i].height = (ymax - ymin) + 1;

		// Centro de Gravidade
		blobs[i].xc = sumx / MAX(blobs[i].area, 1);
		blobs[i].yc = sumy / MAX(blobs[i].area, 1);
	}
	return 1;
}

// dilate
int vc_binary_dilate(IVC *src, IVC *dst, int kernel)
{
	unsigned char *datasrc = (unsigned char *)src->data;
	unsigned char *datadst = (unsigned char *)dst->data;
	int width = src->width;
	int height = src->height;
	int bytesperline = src->bytesperline;
	int channels = src->channels;
	int x, y;
	int xx, yy;
	int xxyymax = (kernel - 1) / 2;
	int xxyymin = -xxyymax;
	int max, min;
	long int pos, posk;
	unsigned char threshold;

	// Verifica��o de erros
	if ((src->width <= 0) || (src->height <= 0) || (src->data == NULL)) return 0;
	if ((src->width != dst->width) || (src->height != dst->height) || (src->channels != dst->channels)) return 0;
	if (channels != 1) return 0;

	int estado = 0;

	for (y = 0; y < height; y++)
	{
		for (x = 0; x < width; x++)
		{
			pos = y * bytesperline + x * channels;

			max = datasrc[pos];
			min = datasrc[pos];

			// NxM Vizinhos
			for (yy = xxyymin; yy <= xxyymax; yy++)
			{
				for (xx = xxyymin; xx <= xxyymax; xx++)
				{
					if ((y + yy >= 0) && (y + yy < height) && (x + xx >= 0) && (x + xx < width))
					{
						posk = (y + yy) * bytesperline + (x + xx) * channels;

						if (datasrc[posk] >= 255) estado = 1;

					}
				}
			}

			if (estado == 1) datadst[pos] = 255;
			else datadst[pos] = 0;
			estado = 0;
		}
	}

	return 1;
}