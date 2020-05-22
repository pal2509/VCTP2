//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//           INSTITUTO POLITÉCNICO DO CÁVADO E DO AVE
//                          2011/2012
//             ENGENHARIA DE SISTEMAS INFORMÁTICOS
//                    VISÃO POR COMPUTADOR
//
//             [  DUARTE DUQUE - dduque@ipca.pt  ]
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

// Desabilita (no MSVC++) warnings de funções não seguras (fopen, sscanf, etc...)
#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <malloc.h>
#include "vc.h"


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//            FUNÇÕES: ALOCAR E LIBERTAR UMA IMAGEM
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++


// Alocar memória para uma imagem
IVC *vc_image_new(int width, int height, int channels, int levels)
{
	IVC *image = (IVC *) malloc(sizeof(IVC));

	if(image == NULL) return NULL;
	if((levels <= 0) || (levels > 255)) return NULL;

	image->width = width;
	image->height = height;
	image->channels = channels;
	image->levels = levels;
	image->bytesperline = image->width * image->channels;
	image->data = (unsigned char *) malloc(image->width * image->height * image->channels * sizeof(char));

	if(image->data == NULL)
	{
		return vc_image_free(image);
	}

	return image;
}


// Libertar memória de uma imagem
IVC *vc_image_free(IVC *image)
{
	if(image != NULL)
	{
		if(image->data != NULL)
		{
			free(image->data);
			image->data = NULL;
		}

		free(image);
		image = NULL;
	}

	return image;
}


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//    FUNÇÕES: LEITURA E ESCRITA DE IMAGENS (PBM, PGM E PPM)
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++


char *netpbm_get_token(FILE *file, char *tok, int len)
{
	char *t;
	int c;
	
	for(;;)
	{
		while(isspace(c = getc(file)));
		if(c != '#') break;
		do c = getc(file);
		while((c != '\n') && (c != EOF));
		if(c == EOF) break;
	}
	
	t = tok;
	
	if(c != EOF)
	{
		do
		{
			*t++ = c;
			c = getc(file);
		} while((!isspace(c)) && (c != '#') && (c != EOF) && (t - tok < len - 1));
		
		if(c == '#') ungetc(c, file);
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

	for(y=0; y<height; y++)
	{
		for(x=0; x<width; x++)
		{
			pos = width * y + x;

			if(countbits <= 8)
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
			if((countbits > 8) || (x == width - 1))
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

	for(y=0; y<height; y++)
	{
		for(x=0; x<width; x++)
		{
			pos = width * y + x;

			if(countbits <= 8)
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
			if((countbits > 8) || (x == width - 1))
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
	if((file = fopen(filename, "rb")) != NULL)
	{
		// Efectua a leitura do header
		netpbm_get_token(file, tok, sizeof(tok));

		if(strcmp(tok, "P4") == 0) { channels = 1; levels = 1; }	// Se PBM (Binary [0,1])
		else if(strcmp(tok, "P5") == 0) channels = 1;				// Se PGM (Gray [0,MAX(level,255)])
		else if(strcmp(tok, "P6") == 0) channels = 3;				// Se PPM (RGB [0,MAX(level,255)])
		else
		{
			#ifdef VC_DEBUG
			printf("ERROR -> vc_read_image():\n\tFile is not a valid PBM, PGM or PPM file.\n\tBad magic number!\n");
			#endif

			fclose(file);
			return NULL;
		}
		
		if(levels == 1) // PBM
		{
			if(sscanf(netpbm_get_token(file, tok, sizeof(tok)), "%d", &width) != 1 || 
			   sscanf(netpbm_get_token(file, tok, sizeof(tok)), "%d", &height) != 1)
			{
				#ifdef VC_DEBUG
				printf("ERROR -> vc_read_image():\n\tFile is not a valid PBM file.\n\tBad size!\n");
				#endif

				fclose(file);
				return NULL;
			}

			// Aloca memória para imagem
			image = vc_image_new(width, height, channels, levels);
			if(image == NULL) return NULL;

			sizeofbinarydata = (image->width / 8 + ((image->width % 8) ? 1 : 0)) * image->height;
			tmp = (unsigned char *) malloc(sizeofbinarydata);
			if(tmp == NULL) return 0;

			#ifdef VC_DEBUG
			printf("\nchannels=%d w=%d h=%d levels=%d\n", image->channels, image->width, image->height, levels);
			#endif

			if((v = fread(tmp, sizeof(unsigned char), sizeofbinarydata, file)) != sizeofbinarydata)
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
			if(sscanf(netpbm_get_token(file, tok, sizeof(tok)), "%d", &width) != 1 || 
			   sscanf(netpbm_get_token(file, tok, sizeof(tok)), "%d", &height) != 1 || 
			   sscanf(netpbm_get_token(file, tok, sizeof(tok)), "%d", &levels) != 1 || levels <= 0 || levels > 255)
			{
				#ifdef VC_DEBUG
				printf("ERROR -> vc_read_image():\n\tFile is not a valid PGM or PPM file.\n\tBad size!\n");
				#endif

				fclose(file);
				return NULL;
			}

			// Aloca memória para imagem
			image = vc_image_new(width, height, channels, levels);
			if(image == NULL) return NULL;

			#ifdef VC_DEBUG
			printf("\nchannels=%d w=%d h=%d levels=%d\n", image->channels, image->width, image->height, levels);
			#endif

			size = image->width * image->height * image->channels;

			if((v = fread(image->data, sizeof(unsigned char), size, file)) != size)
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
	
	if(image == NULL) return 0;

	if((file = fopen(filename, "wb")) != NULL)
	{
		if(image->levels == 1)
		{
			sizeofbinarydata = (image->width / 8 + ((image->width % 8) ? 1 : 0)) * image->height + 1;
			tmp = (unsigned char *) malloc(sizeofbinarydata);
			if(tmp == NULL) return 0;
			
			fprintf(file, "%s %d %d\n", "P4", image->width, image->height);
			
			totalbytes = unsigned_char_to_bit(image->data, tmp, image->width, image->height);
			printf("Total = %ld\n", totalbytes);
			if(fwrite(tmp, sizeof(unsigned char), totalbytes, file) != totalbytes)
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
		
			if(fwrite(image->data, image->bytesperline, image->height, file) != image->height)
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

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//	Paulo Meneses - a17611@alunos.ipca.pt
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//     FUNÇÕES: Algoritmos que trabalham sobre imgens
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++


int vc_gray_negative(IVC* srcdst)
{
	if (srcdst->width <= 0 || srcdst->height <= 0 || srcdst->data == NULL) return 0;
	if (srcdst->channels != 1) return 0;
	for (int i = 0; i < srcdst->bytesperline * srcdst->height ; i++)
	{
		srcdst->data[i] = 255 - srcdst->data[i];
	}
	return srcdst;
}

int vc_rgb_negative(IVC* srcdst)
{
	if (srcdst->width <= 0 || srcdst->height <= 0 || srcdst->data == NULL) return 0;
	if (srcdst->channels != 3) return 0;
	for (int i = 0; i < srcdst->bytesperline * srcdst->height; i++)
	{
		srcdst->data[i] = 255 - srcdst->data[i];
	}
	return srcdst;
}


int vc_rgb_get_red_gray(IVC* srcdst)
{
	if (srcdst->width <= 0 || srcdst->height <= 0 || srcdst->data == NULL) return 0;
	if (srcdst->channels != 3) return 0;
	int pos;
	for (int y = 0; y < srcdst->height; y++)
	{
		for (int x = 0; x < srcdst->width; x++)
		{
			pos = y * srcdst->bytesperline + x * srcdst->channels;
			srcdst->data[pos + 1] = srcdst->data[pos];
			srcdst->data[pos + 2] = srcdst->data[pos];
		}
	}
	
	return srcdst;
}

int vc_rgb_get_green(IVC* srcdst)
{
	if (srcdst->width <= 0 || srcdst->height <= 0 || srcdst->data == NULL) return 0;
	if (srcdst->channels != 3) return 0;

	int pos;
	for (int y = 0; y < srcdst->height; y++)
	{
		for (int x = 0; x < srcdst->width; x++)
		{
			pos = y * srcdst->bytesperline + x * srcdst->channels;
			srcdst->data[pos] = srcdst->data[pos + 1];
			srcdst->data[pos + 2] = srcdst->data[pos + 1];
		}
	}

	return 1;
}

int vc_rgb_get_blue_gray(IVC* srcdst)
{
	if (srcdst->width <= 0 || srcdst->height <= 0 || srcdst->data == NULL) return 0;
	if (srcdst->channels != 3) return 0;

	int pos;
	for (int y = 0; y < srcdst->height; y++)
	{
		for (int x = 0; x < srcdst->width; x++)
		{
			pos = y * srcdst->bytesperline + x * srcdst->channels;
			srcdst->data[pos] = srcdst->data[pos + 2];
			srcdst->data[pos + 1] = srcdst->data[pos + 2];
		}
	}

	return srcdst;
}

//Image conversion from RGB to Gray Scale
int vc_rgb_to_gray(IVC* src, IVC* dst)
{
	////Verifications
	//if (src->width <= 0 || src->height <= 0 || src->data == NULL) return 0;
	//if (src->channels != 3) return 0;
	//if (dst->width <= 0 || dst->height <= 0 || dst->data == NULL) return 0;
	//if (dst->channels != 1) return 0;


	int pos, i;
	for (int y = 0; y < src->height; y++)
	{
		for (int x = 0; x < src->width; x++)
		{
			pos = y * src->bytesperline + x * src->channels;
			i = y * dst->bytesperline + x * dst->channels;
			dst->data[i] = (unsigned char) ((float)src->data[pos + 2]) * 0.299 + ((float)src->data[pos + 1]) * 0.587 + ((float)src->data[pos]) * 0.114;
		}
	}
	
	return 1;
}

int vc_rgb_to_gray_mean(IVC* src, IVC* dst)
{
	//Verifications
	if (src->width <= 0 || src->height <= 0 || src->data == NULL) return 0;
	//if (src->channels != 3) return 0;
	if (dst->width <= 0 || dst->height <= 0 || dst->data == NULL) return 0;
	if (dst->channels != 1) return 0;


	int pos, i;
	for (int y = 0; y < src->height; y++)
	{
		for (int x = 0; x < src->width; x++)
		{
			pos = y * src->bytesperline + x * src->channels;
			i = y * dst->bytesperline + x * dst->channels;
			dst->data[i] = (unsigned char)((((float)src->data[pos]) + ((float)src->data[pos + 1])+ ((float)src->data[pos + 2])) /3);
		}
	}

	return 1;
}


//Converte uma imagem em rgb para hsv
int vc_rgb_to_hsv(IVC* srcdst)
{
	if (srcdst->width <= 0 || srcdst->height <= 0 || srcdst->data == NULL) return 0;
	if (srcdst->channels != 3) return 0;

	int pos;
	float max = 0;
	float min = 0;
	float R = 0, G = 0, B = 0;
	float s = 0;
	float hue = 0;

	for (int y = 0; y < srcdst->height; y++)
	{
		for (int x = 0; x < srcdst->width; x++)
		{
			pos = y * srcdst->bytesperline + x * srcdst->channels;

			R = (float)srcdst->data[pos];
			G = (float)srcdst->data[pos + 1];
			B = (float)srcdst->data[pos + 2];

			max = (R > G ? (R > B ? R : B) : (G > B ? G : B));
			min = (R < G ? (R < B ? R : B) : (G < B ? G : B));
			
			if (max == 0)
			{
				srcdst->data[pos] = 0;
				srcdst->data[pos + 1] = 0;
				srcdst->data[pos + 2] = 0;
			}
			else
			{
				srcdst->data[pos + 2] = (unsigned char)max;

				s = ((max - min) / max) * 255.0f;

				if (s == 0)
				{
					srcdst->data[pos + 1] = 0;
					srcdst->data[pos + 2] = 0;
				}
				else
				{
					srcdst->data[pos + 1] = (unsigned char)s;

					if (max - min == 0) hue = 0;
					else
					{
						if (R > G && R > B && G >= B)
						{

							hue = 60.0f * (G - B) / (max - min);
						}
						if (R > G&& R > B&& G < B)
						{
							hue = 360.0f + 60.0f * (G - B) / (max - min);
						}
						if (R < G && G > B)
						{
							hue = 120.0f + 60.0f * (B - R) / (max - min);
						}
						if (B > G&& R < B)
						{
							hue = 240.0f + 60.0f * (R - G) / (max - min);
						}
					}
					srcdst->data[pos] = (unsigned char)(hue * 255.0f / 360.0f);
				}
			}
		}
	}

	return 1;
}


int vc_hsv_segmentation(IVC* srcdst, int hmin, int hmax, int smin, int smax, int vmin, int vmax)
{
	unsigned char* data = (unsigned char*)srcdst->data;
	int width = srcdst->width;
	int height = srcdst->height;
	int bytesperline = srcdst->bytesperline;
	int channels = srcdst->channels;
	int hue, saturation, value;
	int h, s, v; // h=[0, 360] s=[0, 100] v=[0, 100]
	int i, size;

	// Verificação de erros
	if ((srcdst->width <= 0) || (srcdst->height <= 0) || (srcdst->data == NULL)) return 0;
	if (channels != 3) return 0;

	size = width * height * channels;

	for (i = 0; i < size; i = i + channels)
	{
		h = ((float)data[i]) / 255.0f * 360.0f;
		s = ((float)data[i + 1]) / 255.0f * 100.0f;
		v = ((float)data[i + 2]) / 255.0f * 100.0f;

		if ((h > hmin) && (h <= hmax) && (s >= smin) && (s <= smax) && (v >= vmin) && (v <= vmax))
		{
			data[i] = 255;
			data[i + 1] = 255;
			data[i + 2] = 255;
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


int vc_gray_scale_to_rgb(IVC* src, IVC* dst)
{
	if (src->width <= 0 || src->height <= 0 || src->data == NULL) return 0;
	if (src->channels != 1) return 0;
	if (dst->width <= 0 || dst->height <= 0 || dst->data == NULL) return 0;
	if (dst->channels != 3) return 0;

	int srcpos, dstpos;
	for (int y = 0; y < src->height; y++)
	{
		for (int x = 0; x < src->width; x++)
		{
			srcpos = y * src->bytesperline + x * src->channels;
			dstpos = y * dst->bytesperline + x * dst->channels;
			
			if (src->data[srcpos] >= 192) {
				dst->data[dstpos] = 255;
				dst->data[dstpos + 1] = src->data[srcpos] * 192 / 255;
				dst->data[dstpos + 2] = 0;
			}
			if (128 >= src->data[srcpos] < 192) {
				dst->data[dstpos] = src->data[srcpos];
				dst->data[dstpos + 1] = 255;
				dst->data[dstpos + 2] = 0;
			}
			if (64 >= src->data[srcpos] < 128) {
				dst->data[dstpos] = 0;
				dst->data[dstpos + 1] = 255;
				dst->data[dstpos + 2] = src->data[srcpos];
			}
			if (0 >= src->data[srcpos] < 64) {
				dst->data[dstpos] = 0;
				dst->data[dstpos + 1] = src->data[srcpos];
				dst->data[dstpos + 2] = 255;
			}
		}
	}

	return 1;
}

//Conversão de uma imagem em escale de cizento em binario por threshold
int vc_gray_to_binary(IVC* srcdst, int threshold)
{
	if (srcdst->width <= 0 || srcdst->height <= 0 || srcdst->data == NULL) return 0;
	if (srcdst->channels != 1) return 0;


	int pos;
	for (int y = 0; y < srcdst->height; y++)
	{
		for (int x = 0; x < srcdst->width; x++)
		{
			pos = y * srcdst->bytesperline + x * srcdst->channels;

			if (srcdst->data[pos] > threshold)srcdst->data[pos] = 255;
			else srcdst->data[pos] = 0;
		}
	}

	return 1;
}

int vc_bin_negative(IVC* srcdst)
{
	if (srcdst->width <= 0 || srcdst->height <= 0 || srcdst->data == NULL) return 0;
	if (srcdst->channels != 1) return 0;
	for (int i = 0; i < srcdst->height * srcdst->width; i++)
	{
		if (srcdst->data[i] == 255)srcdst->data[i] = 0;
		else srcdst->data[i] = 255;
	}
}


//Conversão de uma imagem em escala de  cizentos para binária através da media global da cor
int vc_gray_to_binary_global_mean(IVC* srcdst)
{
	if (srcdst->width <= 0 || srcdst->height <= 0 || srcdst->data == NULL) return 0;
	if (srcdst->channels != 1) return 0;


	int pos, pixels = srcdst->height * srcdst->width;
	float sum = 0;
	for (int y = 0; y < srcdst->height; y++)
	{
		for (int x = 0; x < srcdst->width; x++)
		{
			pos = y * srcdst->bytesperline + x * srcdst->channels;
			sum = sum + srcdst->data[pos];
		}
	}
	float mean = sum / pixels;

	for (int y = 0; y < srcdst->height; y++)
	{
		for (int x = 0; x < srcdst->width; x++)
		{
			pos = y * srcdst->bytesperline + x * srcdst->channels;
			if (srcdst->data[pos] > mean) srcdst->data[pos] = 255;
			else srcdst->data[pos] = 0;
		}
	}

	return 1;
}

// Converter de Gray para Binário (threshold automático Midpoint)
int vc_gray_to_binary_midpoint(IVC* src, IVC* dst, int kernel)
{
	// Verificação de erros
	if ((src->width <= 0) || (src->height <= 0) || (src->data == NULL)) return 0;
	if ((src->width != dst->width) || (src->height != dst->height) || (src->channels != dst->channels)) return 0;
	if (src->channels != 1) return 0;

	int pos, posk;
	float max = 255, min = 0;
	int offset = (kernel - 1) / 2; //(int) floor(((double) kernel) / 2.0);
	int ky, kx;
	unsigned char threshold;

	for (int y = 0; y < src->height; y++)
	{
		for (int x = 0; x < src->width; x++)
		{
			pos = y * src->bytesperline + x * src->channels;//Calculo da posição atual no array de pixeis;

			max = 0;
			min = 255;

			for (ky = -offset; ky < offset; ky++)
			{
				for (kx = -offset; kx < offset; kx++)
				{
					if ((y + ky >= 0) && (y + ky <= src->height) && (x + kx >= 0) && (x + kx < src->width))
					{
						posk = (y + ky) * src->bytesperline + (x + kx) * src->channels;//Calculo da posição de offset em relação ao pixel que estamos;
						
						if (src->data[posk] > max) max = src->data[posk];
						if (src->data[posk] < min) min = src->data[posk];

					}
				}
			}

			threshold = (unsigned char)((float)1 / 2 * (min + max));//Calculo do threshold;

			if (src->data[pos] > threshold)dst->data[pos] = 255;
			else dst->data[pos] = 0;

		}
	}

	return 1;
}

// Converter de Gray para Binário (threshold automático Niblack)
int vc_gray_to_binary_niblack(IVC* src, IVC* dst, int kernel, int cmin)
{
	// Verificação de erros
	if ((src->width <= 0) || (src->height <= 0) || (src->data == NULL)) return 0;
	if ((src->width != dst->width) || (src->height != dst->height) || (src->channels != dst->channels)) return 0;
	if (src->channels != 1) return 0;

	int pos, posk;
	float max = 255, min = 0;
	int offset = (kernel - 1) / 2; //(int) floor(((double) kernel) / 2.0);
	int ky, kx;
	unsigned char threshold;

	for (int y = 0; y < src->height; y++)
	{
		for (int x = 0; x < src->width; x++)
		{
			pos = y * src->bytesperline + x * src->channels;//Calculo da posição atual no array de pixeis;

			max = 0;
			min = 255;

			for (ky = -offset; ky < offset; ky++)
			{
				for (kx = -offset; kx < offset; kx++)
				{
					if ((y + ky >= 0) && (y + ky <= src->height) && (x + kx >= 0) && (x + kx < src->width))
					{
						posk = (y + ky) * src->bytesperline + (x + kx) * src->channels;//Calculo da posição de offset em relação ao pixel que estamos;

						if (src->data[posk] > max) max = src->data[posk];
						if (src->data[posk] < min) min = src->data[posk];

					}
				}
			}



		}
	}

	return 1;
}



//Dilatação binária de uma imagem
int vc_binary_dilate(IVC* src, IVC* dst, int kernel)
{
	// Verificação de erros
	if ((src->width <= 0) || (src->height <= 0) || (src->data == NULL)) return 0;
	if ((src->width != dst->width) || (src->height != dst->height) || (src->channels != dst->channels)) return 0;
	if (src->channels != 1) return 0;

	int pos, posk;
	int offset = (kernel - 1) / 2;
	int x, y, kx, ky;
	int val = 0;
	int max = src->width * src->height;
	for (y = 0; y < src->height; y++)
	{
		for (x = 0; x < src->width; x++)
		{
			pos = y * src->bytesperline + x * src->channels;//Calculo da posição atual no array de pixeis;

			for (ky = -offset; ky < offset; ky++)
			{
				for (kx = -offset; kx < offset; kx++)
				{
					if ((y + ky >= 0) && (y + ky <= src->height) && (x + kx >= 0) && (x + kx < src->width))
					{
						posk = (y + ky) * src->bytesperline + (x + kx) * src->channels;//Calculo da posição de offset em relação ao pixel que estamos;
						if (posk <= max)
						{
							if (src->data[posk] == 255)val = 1;
						}
					}
				}
			}

			if (val == 1)dst->data[pos] = 255;
			else dst->data[pos] = 0;
			val = 0;
		}
	}
	return 1;

}

//Erosão de um imagem binária
int vc_binary_erode(IVC* src, IVC* dst, int kernel)
{
	// Verificação de erros
	if ((src->width <= 0) || (src->height <= 0) || (src->data == NULL)) return 0;
	if ((src->width != dst->width) || (src->height != dst->height) || (src->channels != dst->channels)) return 0;
	if (src->channels != 1) return 0;

	int pos, posk;
	int offset = (kernel - 1) / 2;
	int x, y, kx, ky;
	int val = 0;

	for (y = 0; y < src->height; y++)
	{
		for (x = 0; x < src->width; x++)
		{
			pos = y * src->bytesperline + x * src->channels;//Calculo da posição atual no array de pixeis;

			for (ky = -offset; ky < offset; ky++)
			{
				for (kx = -offset; kx < offset; kx++)
				{
					if ((y + ky >= 0) && (y + ky <= src->height) && (x + kx >= 0) && (x + kx < src->width))
					{
						posk = (y + ky) * src->bytesperline + (x + kx) * src->channels;//Calculo da posição de offset em relação ao pixel que estamos;

						if (posk < src->height * src->width && src->data[posk] == 0 )val = 1;
					}
				}
			}

			if (val == 1)dst->data[pos] = 0;
			else dst->data[pos] = 255;
			val = 0;

		}
	}
	return 1;
}


int vc_image_dupp(IVC* src, IVC* dst)
{
	if ((src->width <= 0) || (src->height <= 0) || (src->data == NULL)) return 0;
	if ((src->width != dst->width) || (src->height != dst->height) || (src->channels != dst->channels)) return 0;


	int pos;
	for (int y = 0; y < src->height; y++)
	{
		for (int x = 0; x < src->width; x++)
		{
			pos = y * src->bytesperline + x * src->channels;//Calculo da posição atual no array de pixeis;

			dst->data[pos] = src->data[pos];

		}
	}

	return 1;
}


int vc_binary_open(IVC* src, IVC* dst, int kernel)
{
	if ((src->width <= 0) || (src->height <= 0) || (src->data == NULL)) return 0;
	if ((src->width != dst->width) || (src->height != dst->height) || (src->channels != dst->channels)) return 0;
	if (src->channels != 1) return 0;

	IVC* new = vc_image_new(src->width, src->height, src->channels, src->levels);
	vc_binary_erode(src, new, kernel);
	vc_binary_dilate(new, dst, kernel);
	free(new);
	
	return 1;
}


int vc_binary_close(IVC* src, IVC* dst, int kernel)
{
	if ((src->width <= 0) || (src->height <= 0) || (src->data == NULL)) return 0;
	if ((src->width != dst->width) || (src->height != dst->height) || (src->channels != dst->channels)) return 0;
	if (src->channels != 1) return 0;


	IVC* new = vc_image_new(src->width, src->height, src->channels, src->levels);
	vc_binary_dilate(src, new, kernel);
	vc_binary_erode(new, dst, kernel);
	free(new);

	return 1;
}


int vc_grayscale_dilate(IVC* src, IVC* dst, int kernel)
{
	if ((src->width <= 0) || (src->height <= 0) || (src->data == NULL)) return 0;
	if ((src->width != dst->width) || (src->height != dst->height) || (src->channels != dst->channels)) return 0;
	if (src->channels != 1) return 0;


	int pos, posk;
	int offset = (kernel - 1) / 2;
	int x, y, kx, ky;
	unsigned char max = 0;

	for (y = 0; y < src->height; y++)
	{
		for (x = 0; x < src->width; x++)
		{
			pos = y * src->bytesperline + x * src->channels;//Calculo da posição atual no array de pixeis;

			max = 0;

			for (ky = -offset; ky < offset; ky++)
			{
				for (kx = -offset; kx < offset; kx++)
				{
					if ((y + ky >= 0) && (y + ky <= src->height) && (x + kx >= 0) && (x + kx < src->width))
					{
						posk = (y + ky) * src->bytesperline + (x + kx) * src->channels;//Calculo da posição de offset em relação ao pixel que estamos;
			
						if (src->data[posk] > max)max = src->data[posk];
					}
				}
			}

			dst->data[pos] = max;

		}
	}

	return 1;
}

int vc_grayscale_erode(IVC* src, IVC* dst, int kernel)
{
	if ((src->width <= 0) || (src->height <= 0) || (src->data == NULL)) return 0;
	if ((src->width != dst->width) || (src->height != dst->height) || (src->channels != dst->channels)) return 0;
	if (src->channels != 1) return 0;


	int pos, posk;
	int offset = (kernel - 1) / 2;
	int x, y, kx, ky;
	int min = 255;

	for (y = 0; y < src->height; y++)
	{
		for (x = 0; x < src->width; x++)
		{
			pos = y * src->bytesperline + x * src->channels;//Calculo da posição atual no array de pixeis;

			min = 255;

			for (ky = -offset; ky < offset; ky++)
			{
				for (kx = -offset; kx < offset; kx++)
				{
					if ((y + ky >= 0) && (y + ky <= src->height) && (x + kx >= 0) && (x + kx < src->width))
					{
						posk = (y + ky) * src->bytesperline + (x + kx) * src->channels;//Calculo da posição de offset em relação ao pixel que estamos;

						if (src->data[posk] < min)min = src->data[posk];
					}
				}
			}

			dst->data[pos] = (unsigned char)min;
		}
	}

	return 1;
}

int vc_grayscale_open(IVC* src, IVC* dst, int kernel)
{
	if ((src->width <= 0) || (src->height <= 0) || (src->data == NULL)) return 0;
	if ((src->width != dst->width) || (src->height != dst->height) || (src->channels != dst->channels)) return 0;
	if (src->channels != 1) return 0;

	IVC* new = vc_image_new(src->width, src->height, src->channels, src->levels);
	vc_grayscale_erode(src, new, kernel);
	vc_grayscale_dilate(new, dst, kernel);
	free(new);

	return 1;
}

int vc_grayscale_close(IVC* src, IVC* dst, int kernel)
{
	if ((src->width <= 0) || (src->height <= 0) || (src->data == NULL)) return 0;
	if ((src->width != dst->width) || (src->height != dst->height) || (src->channels != dst->channels)) return 0;
	if (src->channels != 1) return 0;


	IVC* new = vc_image_new(src->width, src->height, src->channels, src->levels);
	vc_grayscale_dilate(src, new, kernel);
	vc_grayscale_erode(new, dst, kernel);
	free(new);

	return 1;
}

// Etiquetagem de blobs
// src		: Imagem bin�ria de entrada
// dst		: Imagem grayscale (ir� conter as etiquetas)
// nlabels	: Endere�o de mem�ria de uma vari�vel, onde ser� armazenado o n�mero de etiquetas encontradas.
// OVC*		: Retorna um array de estruturas de blobs (objectos), com respectivas etiquetas. � necess�rio libertar posteriormente esta mem�ria.
OVC* vc_binary_blob_labelling(IVC* src, IVC* dst, int* nlabels)
{
	unsigned char* datasrc = (unsigned char*)src->data;
	unsigned char* datadst = (unsigned char*)dst->data;
	int width = src->width;
	int height = src->height;
	int bytesperline = src->bytesperline;
	int channels = src->channels;
	int x, y, a, b;
	long int i, size;
	long int posX, posA, posB, posC, posD;
	int labeltable[5000] = { 0 };
	int labelarea[5000] = { 0 };
	int label = 1; // Etiqueta inicial.
	int num, tmplabel;
	OVC* blobs; // Apontador para array de blobs (objectos) que será retornado desta função.

				// Verificação de erros
	if ((src->width <= 0) || (src->height <= 0) || (src->data == NULL)) return 0;
	if ((src->width != dst->width) || (src->height != dst->height) || (src->channels != dst->channels)) return NULL;
	if (channels != 1) return NULL;

	// Copia dados da imagem bin�ria para imagem grayscale
	memcpy(datadst, datasrc, bytesperline * height);

	// Todos os pixéis de plano de fundo devem obrigatóriamente ter valor 0
	// Todos os pixéis de primeiro plano devem obrigatóriamente ter valor 255
	// Serão atribuídas etiquetas no intervalo [1,254]
	// Este algoritmo está assim limitado a 255 labels
	for (i = 0, size = bytesperline * height; i < size; i++)
	{
		if (datadst[i] != 0) datadst[i] = 255;
	}

	// Limpa os rebordos da imagem binária
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

					// Se A está marcado
					if (datadst[posA] != 0) num = labeltable[datadst[posA]];
					// Se B está marcado, e � menor que a etiqueta "num"
					if ((datadst[posB] != 0) && (labeltable[datadst[posB]] < num)) num = labeltable[datadst[posB]];
					// Se C está marcado, e � menor que a etiqueta "num"
					if ((datadst[posC] != 0) && (labeltable[datadst[posC]] < num)) num = labeltable[datadst[posC]];
					// Se D está marcado, e � menor que a etiqueta "num"
					if ((datadst[posD] != 0) && (labeltable[datadst[posD]] < num)) num = labeltable[datadst[posD]];

					// Atribui a etiqueta ao pixel
					datadst[posX] = num;
					labeltable[num] = num;

					// Actualiza a tabela de etiquetas
					if (datadst[posA] != 0)
					{
						if (labeltable[datadst[posA]] != num)
						{
							for (tmplabel = labeltable[datadst[posA]], a = 1; a < label; a++)
							{
								if (labeltable[a] == tmplabel)
								{
									labeltable[a] = num;
								}
							}
						}
					}
					if (datadst[posB] != 0)
					{
						if (labeltable[datadst[posB]] != num)
						{
							for (tmplabel = labeltable[datadst[posB]], a = 1; a < label; a++)
							{
								if (labeltable[a] == tmplabel)
								{
									labeltable[a] = num;
								}
							}
						}
					}
					if (datadst[posC] != 0)
					{
						if (labeltable[datadst[posC]] != num)
						{
							for (tmplabel = labeltable[datadst[posC]], a = 1; a < label; a++)
							{
								if (labeltable[a] == tmplabel)
								{
									labeltable[a] = num;
								}
							}
						}
					}
					if (datadst[posD] != 0)
					{
						if (labeltable[datadst[posD]] != num)
						{
							for (tmplabel = labeltable[datadst[posC]], a = 1; a < label; a++)
							{
								if (labeltable[a] == tmplabel)
								{
									labeltable[a] = num;
								}
							}
						}
					}
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

	//printf("\nMax Label = %d\n", label);

	// Contagem do n�mero de blobs
	// Passo 1: Eliminar, da tabela, etiquetas repetidas
	for (a = 1; a < label - 1; a++)
	{
		for (b = a + 1; b < label; b++)
		{
			if (labeltable[a] == labeltable[b]) labeltable[b] = 0;
		}
	}
	// Passo 2: Conta etiquetas e organiza a tabela de etiquetas, para que n�o hajam valores vazios (zero) entre etiquetas
	*nlabels = 0;
	for (a = 1; a < label; a++)
	{
		if (labeltable[a] != 0)
		{
			labeltable[*nlabels] = labeltable[a]; // Organiza tabela de etiquetas
			(*nlabels)++; // Conta etiquetas
		}
	}

	// Se n�o h� blobs
	if (*nlabels == 0) return NULL;

	// Cria lista de blobs (objectos) e preenche a etiqueta
	blobs = (OVC*)calloc((*nlabels), sizeof(OVC));
	if (blobs != NULL)
	{
		for (a = 0; a < (*nlabels); a++) blobs[a].label = labeltable[a];
	}
	else return NULL;

	return blobs;
}


int vc_binary_blob_info(IVC* src, OVC* blobs, int nblobs)
{
	unsigned char* data = (unsigned char*)src->data;
	int width = src->width;
	int height = src->height;
	int bytesperline = src->bytesperline;
	int channels = src->channels;
	int x, y, i;
	long int pos;
	int xmin, ymin, xmax, ymax;
	long int sumx, sumy;

	// Verifica��o de erros
	if ((src->width <= 0) || (src->height <= 0) || (src->data == NULL)) return 0;
	if (channels != 1) return 0;

	// Conta �rea de cada blob
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
					// �rea
					blobs[i].area++;

					// Centro de Gravidade
					sumx += x;
					sumy += y;

					// Bounding Box
					if (xmin > x) xmin = x;
					if (ymin > y) ymin = y;
					if (xmax < x) xmax = x;
					if (ymax < y) ymax = y;

					// Per�metro
					// Se pelo menos um dos quatro vizinhos n�o pertence ao mesmo label, ent�o � um pixel de contorno
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
		//blobs[i].xc = (xmax - xmin) / 2;
		//blobs[i].yc = (ymax - ymin) / 2;
		blobs[i].xc = sumx / MAX(blobs[i].area, 1);
		blobs[i].yc = sumy / MAX(blobs[i].area, 1);
	}

	return 1;
}

// Converter de Gray para Binário (threshold automático Bersen)
int vc_gray_to_binary_bernsen(IVC* src, IVC* dst, int kernel, int cmin)
{
	// Verificação de erros
	if ((src->width <= 0) || (src->height <= 0) || (src->data == NULL)) return 0;
	if ((src->width != dst->width) || (src->height != dst->height) || (src->channels != dst->channels)) return 0;
	if (src->channels != 1) return 0;

	int pos, posk;
	float max = 255, min = 0;
	int offset = (kernel - 1) / 2; //(int) floor(((double) kernel) / 2.0);
	int ky, kx;
	unsigned char threshold;
	int m = src->width * src->height;

	for (int y = 0; y < src->height; y++)
	{
		for (int x = 0; x < src->width; x++)
		{
			pos = y * src->bytesperline + x * src->channels;//Calculo da posição atual no array de pixeis;

			max = 0;
			min = 255;

			for (ky = -offset; ky < offset; ky++)
			{
				for (kx = -offset; kx < offset; kx++)
				{
					if ((y + ky >= 0) && (y + ky <= src->height) && (x + kx >= 0) && (x + kx < src->width))
					{
						posk = (y + ky) * src->bytesperline + (x + kx) * src->channels;//Calculo da posição de offset em relação ao pixel que estamos;

						if (posk <= m)
						{
							if (src->data[posk] > max) max = src->data[posk];
							if (src->data[posk] < min) min = src->data[posk];
							
						}

					}
				}
			}

			if ((max - min) < cmin) threshold = (unsigned char)(src->levels / 2);
			else threshold = (unsigned char)((min + max) / 2);

			if (src->data[pos] > threshold && pos <= m)dst->data[pos] = 255;
			else dst->data[pos] = 0;

		}
	}

	return 1;
}

// Detecção de contornos pelos operadores Prewitt
int vc_gray_edge_prewitt(IVC* src, IVC* dst, float th) // th = [0.001, 1.000]
{
	unsigned char* datasrc = (unsigned char*)src->data;
	unsigned char* datadst = (unsigned char*)dst->data;
	int width = src->width;
	int height = src->height;
	int bytesperline = src->bytesperline;
	int channels = src->channels;
	int x, y;
	long int posX, posA, posB, posC, posD, posE, posF, posG, posH;
	int i, size;
	int histmax, histthreshold;
	int sumx, sumy;
	int hist[256] = { 0 };

	// Verificação de erros
	if ((src->width <= 0) || (src->height <= 0) || (src->data == NULL)) return 0;
	if ((src->width != dst->width) || (src->height != dst->height) || (src->channels != dst->channels)) return 0;
	if (channels != 1) return 0;

	size = width * height;

	for (y = 1; y < height - 1; y++)
	{
		for (x = 1; x < width - 1; x++)
		{
			posA = (y - 1) * bytesperline + (x - 1) * channels;
			posB = (y - 1) * bytesperline + x * channels;
			posC = (y - 1) * bytesperline + (x + 1) * channels;
			posD = y * bytesperline + (x - 1) * channels;
			posX = y * bytesperline + x * channels;
			posE = y * bytesperline + (x + 1) * channels;
			posF = (y + 1) * bytesperline + (x - 1) * channels;
			posG = (y + 1) * bytesperline + x * channels;
			posH = (y + 1) * bytesperline + (x + 1) * channels;

			sumx = datasrc[posA] * -1;
			sumx += datasrc[posD] * -1;
			sumx += datasrc[posF] * -1;

			sumx += datasrc[posC] * +1;
			sumx += datasrc[posE] * +1;
			sumx += datasrc[posH] * +1;
			sumx = sumx / 3; // 3 = 1 + 1 + 1

			sumy = datasrc[posA] * -1;
			sumy += datasrc[posB] * -1;
			sumy += datasrc[posC] * -1;

			sumy += datasrc[posF] * +1;
			sumy += datasrc[posG] * +1;
			sumy += datasrc[posH] * +1;
			sumy = sumy / 3; // 3 = 1 + 1 + 1

			datadst[posX] = (unsigned char)sqrt((double)(sumx * sumx + sumy * sumy));
		}
	}

	// Compute a grey level histogram
	for (y = 0; y < height; y++)
	{
		for (x = 0; x < width; x++)
		{
			hist[datadst[y * bytesperline + x * channels]]++;
		}
	}

	// Threshold at the middle of the occupied levels
	histmax = 0;
	for (i = 0; i <= 255; i++)
	{
		histmax += hist[i];

		// th = Prewitt Threshold
		if (histmax >= (((float)size) * th)) break;
	}
	histthreshold = i;

	// Apply the threshold
	for (y = 0; y < height; y++)
	{
		for (x = 0; x < width; x++)
		{
			posX = y * bytesperline + x * channels;

			if (datadst[posX] >= histthreshold) datadst[posX] = 255;
			else datadst[posX] = 0;
		}
	}

	return 1;
}