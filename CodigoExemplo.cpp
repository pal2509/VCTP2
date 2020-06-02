#include <iostream>
#include <string>
#include <chrono>
#include <opencv2\opencv.hpp>
#include <opencv2\core.hpp>
#include <opencv2\highgui.hpp>
#include <opencv2\videoio.hpp>

extern "C" {
#include <string.h>
#include "vc.h"
}



int BoundingBox(IVC* src);
void Frame(IVC* frame, char *c);
int* GetRowProf(IVC* image, int y);
int* GetColunmProf(IVC* image);
void mediaMovel(double* histograma, int n);
double* HistogramaHorizontalF(IVC* image);
double* HistogramaVerticalF(IVC* image);
void normalizaHist(double* histograma, int n, int totalPixeis);
char IdentificaCaracter(IVC* image);
int* IndexPicos(double* histograma, int n, int* npicos);
float Media(double* hist, int n);
int CalculaMediaDesvio(IVC* image, double* media, double* desvio);
int vhistogram(int h[], int n, int kernel, int cmin, bool* a);


//Histograma horizontal de uma imagem binario
int* BinHHist(IVC* image)
{
	int* h = (int*)calloc(image->height, sizeof(int));
	int pos = 0;
	for (int y = 0; y < image->height; y++)
	{

		for (int x = 0; x < image->width; x++)
		{
			pos = y * image->bytesperline + x;

			if (image->data[pos] == (unsigned char)255)h[y] = h[y] + 1;
		}

	}

	return h;
}

//Histograma horizontal de uma imagem binario
int* BinVHist(IVC* image)
{
	int* v = (int*)calloc(image->width, sizeof(int));
	int pos = 0;
	for (int x = 0; x < image->width; x++)
	{
		for (int y = 0; y < image->height; y++)
		{
			pos = y * image->bytesperline + x;

			if (image->data[pos] == 255)v[x] = v[x] + 1;
		}

	}
	return v;
}

void vc_timer(void) {
	static bool running = false;
	static std::chrono::steady_clock::time_point previousTime = std::chrono::steady_clock::now();
	if (!running) {
		running = true;
	}
	else {
		std::chrono::steady_clock::time_point currentTime = std::chrono::steady_clock::now();
		std::chrono::steady_clock::duration elapsedTime = currentTime - previousTime;

		// Tempo em segundos.
		std::chrono::duration<double> time_span = std::chrono::duration_cast<std::chrono::duration<double>>(elapsedTime);
		double nseconds = time_span.count();

		std::cout << "Tempo decorrido: " << nseconds << "segundos" << std::endl;
		std::cout << "Pressione qualquer tecla para continuar...\n";
		std::cin.get();
	}
}




int main(void) {
	// Vídeo
	char videofile[37] = "VC-TP2 - Enunciado\\Videos\\video1.mp4";
	cv::VideoCapture capture;
	struct
	{
		int width, height;
		int ntotalframes;
		int fps;
		int nframe;
	} video;
	// Outros
	std::string str;
	int key = 0;

	/* Leitura de vídeo de um ficheiro */
	/* NOTA IMPORTANTE:
	O ficheiro video.avi deverá estar localizado no mesmo directório que o ficheiro de código fonte.
	*/
	capture.open(videofile);
	
	/* Em alternativa, abrir captura de vídeo pela Webcam #0 */
	//capture.open(0, cv::CAP_DSHOW); // Pode-se utilizar apenas capture.open(0);
	
	/* Verifica se foi possível abrir o ficheiro de vídeo */
	if (!capture.isOpened())
	{
		std::cerr << "Erro ao abrir o ficheiro de vídeo!\n";
		return 1;
	}



	/* Número total de frames no vídeo */
	video.ntotalframes = (int)capture.get(cv::CAP_PROP_FRAME_COUNT);
	/* Frame rate do vídeo */
	video.fps = (int)capture.get(cv::CAP_PROP_FPS);
	/* Resolução do vídeo */
	video.width = (int)capture.get(cv::CAP_PROP_FRAME_WIDTH);
	video.height = (int)capture.get(cv::CAP_PROP_FRAME_HEIGHT);

	/* Cria uma janela para exibir o vídeo */
	cv::namedWindow("VC - VIDEO", cv::WINDOW_AUTOSIZE);

	/* Inicia o timer */
	vc_timer();
	

	cv::Mat frame;
	while (key != 'q') {
		/* Leitura de uma frame do vídeo */
		capture.read(frame);

		/* Verifica se conseguiu ler a frame */
		if (frame.empty()) break;

		/* Número da frame a processar */
		video.nframe = (int)capture.get(cv::CAP_PROP_POS_FRAMES);



		// Faça o seu código aqui...
		// Cria uma nova imagem IVC
		IVC* image = vc_image_new(video.width, video.height, 3, 255);


		// Copia dados de imagem da estrutura cv::Mat para uma estrutura IVC
		memcpy(image->data, frame.data, video.width * video.height * 3);
		// Executa uma função da nossa biblioteca vc
		//vc_rgb_get_green(image);
		//BoundingBox(image);
		char *matricula = (char*)calloc(6,sizeof(char));
		try {
			Frame(image, matricula);
		}
		catch(std::exception e)
		{ }

		// Copia dados de imagem da estrutura IVC para uma estrutura cv::Mat
		memcpy(frame.data, image->data, video.width * video.height * 3);
		// Liberta a memória da imagem IVC que havia sido criada
		vc_image_free(image);
		// +++++++++++++++++++++++++
				/* Exemplo de inserção texto na frame */
		str = std::string("RESOLUCAO: ").append(std::to_string(video.width)).append("x").append(std::to_string(video.height));
		cv::putText(frame, str, cv::Point(20, 25), cv::FONT_HERSHEY_SIMPLEX, 1.0, cv::Scalar(0, 0, 0), 2);
		cv::putText(frame, str, cv::Point(20, 25), cv::FONT_HERSHEY_SIMPLEX, 1.0, cv::Scalar(255, 255, 255), 1);
		str = std::string("TOTAL DE FRAMES: ").append(std::to_string(video.ntotalframes));
		cv::putText(frame, str, cv::Point(20, 50), cv::FONT_HERSHEY_SIMPLEX, 1.0, cv::Scalar(0, 0, 0), 2);
		cv::putText(frame, str, cv::Point(20, 50), cv::FONT_HERSHEY_SIMPLEX, 1.0, cv::Scalar(255, 255, 255), 1);
		str = std::string("FRAME RATE: ").append(std::to_string(video.fps));
		cv::putText(frame, str, cv::Point(20, 75), cv::FONT_HERSHEY_SIMPLEX, 1.0, cv::Scalar(0, 0, 0), 2);
		cv::putText(frame, str, cv::Point(20, 75), cv::FONT_HERSHEY_SIMPLEX, 1.0, cv::Scalar(255, 255, 255), 1);
		str = std::string("N. DA FRAME: ").append(std::to_string(video.nframe));
		cv::putText(frame, str, cv::Point(20, 100), cv::FONT_HERSHEY_SIMPLEX, 1.0, cv::Scalar(0, 0, 0), 2);
		cv::putText(frame, str, cv::Point(20, 100), cv::FONT_HERSHEY_SIMPLEX, 1.0, cv::Scalar(255, 255, 255), 1);
		//Escrita da matricula no frame
		std::string m;
		std::stringstream ss;
		ss << matricula;
		ss >> m;
		cv::putText(frame, m, cv::Point(20, 125), cv::FONT_HERSHEY_SIMPLEX, 1.0, cv::Scalar(0, 0, 0), 2);
		cv::putText(frame, m, cv::Point(20, 125), cv::FONT_HERSHEY_SIMPLEX, 1.0, cv::Scalar(255, 255, 255), 1);

		/* Exibe a frame */
		cv::imshow("VC - VIDEO", frame);

		/* Sai da aplicação, se o utilizador premir a tecla 'q' */
		if (cv::waitKey(1) == 'q')
			while (cv::waitKey(1) != 'q');
	}


	/* Para o timer e exibe o tempo decorrido */
	vc_timer();
	
	/* Fecha a janela */
	cv::destroyWindow("VC - VIDEO");

	/* Fecha o ficheiro de vídeo */
	capture.release();

	return 0;
}

//Funçao para a analise de um frame e desenho das caixas delimitadoras da matricula e das letras
void Frame(IVC* frame, char* c)
{
	IVC* frameg = vc_image_new(frame->width, frame->height, 1, 255);//imagem para o frame
	vc_rgb_to_gray(frame, frameg);//conversão de rgb para binário
	//vc_write_image((char*)"frameg.pgm", frameg);

	int *h;
	int pos = 0;
	
	int lmc = frame->width * 0.05;//largura minima do caracter
	int amc = frame->height * 0.0001 * frame->width;//altura minima do caracter
	int width = frame->width;//largura do frame
	int ymin = 0;
	int ymax = 0;
	//Ciclo para percorrer o frame linha a linha com incrementos de amc
	for (int y = 0; y < frameg->height; y = y + amc)
	{
		int cmin = 100;
		h = GetRowProf(frameg, y);//histograma em escala de cinzento de uma determinada linha
		bool proximidade = false;
		int v = vhistogram(h, width, lmc, cmin, &proximidade);//Divide o histograma em partes iguas e faz a diferença de contrates dentro delas e conta as
			//diferençãs que são superiores a 100

		//for (int x = 0; x < frame->width; x++)
		//{
		//	pos = y * frame->bytesperline + x * frame->channels;
		//	frame->data[pos] = (unsigned char)255;
		//	frame->data[pos + 1] = (unsigned char)255;
		//	frame->data[pos + 2] = (unsigned char)255;
		//}
		//for (int i = 0; i < width; i++)
		//{
		//	printf("%d->%d\n",i, h[i]);			
		//}
		//printf("V = %d\n", v);
		//printf("P = %d\n", proximidade);
		if (v >= 6 && v <= 13 && proximidade)//se houver entre 6 a 13 essa linha interceta a matricula e serem todas consecutivas
		{
			//for (int x = 0; x < frame->width; x++)
			//{
			//	pos = y * frame->bytesperline + x * frame->channels;
			//	frame->data[pos] = (unsigned char)255;
			//	frame->data[pos + 1] = (unsigned char)255;
			//	frame->data[pos + 2] = (unsigned char)255;
			//}

			int* yh;
			int yv = 0;
			int i = y - amc * 2;
			proximidade = false;
			//ciclo para encontrar o extremo superior da matricula
			do
			{
				yh = GetRowProf(frameg,i);
				yv = vhistogram(yh, width, lmc, cmin,&proximidade);
				//printf("%d\n", yv);
				if (yv >= 6 && yv <= 13 && proximidade)
				{
					if (i < y && i >= 10)
					{
						ymin = i - lmc * 0.3;
						free(yh);
						break;
					}
				}
				i++;
				free(yh);
			} while (i < y);

			
			//ciclo para encontrar o extremo inferior da matricula
			i = y + amc * 2;
			proximidade = false;
			do
			{
				yh = GetRowProf(frameg, i);
				yv = vhistogram(yh, width, lmc, cmin, &proximidade);
				//printf("%d\n", yv);
				if (yv >= 6 && yv <= 13 && proximidade)
				{
					if (i > y && i >= 0)
					{
						ymax = i + lmc * 0.3;
						free(yh);
						break;
					}
				}
				free(yh);
				i--;
			} while (i > y);

			//for (int x = 0; x < frame->width; x++)
			//{
			//	pos = y * frame->bytesperline + x * frame->channels;
			//	frame->data[pos] = (unsigned char)255;
			//	frame->data[pos + 1] = (unsigned char)255;
			//	frame->data[pos + 2] = (unsigned char)255;
			//}
		}
		free(h);		
	}

	int height = ymax - ymin;//Calcular a altura da matricula
	int xmin = 0, xmax = 0;
	
	if (height > 10 && height < 300)
	{
		IVC* row = vc_image_new(frameg->width, height, 1, 255);//nova imagem para essa linha toda aonde está a matricula

		//Copiar essa linha da imagem original em escala de cinzento para a imagem row
		int posk = 0, yk = 0;
		for (int y = ymin; y < ymax; y++)
		{
			for (int x = 0; x < row->width; x++)
			{
				pos = y * row->width + x;
				posk = yk * row->width + x;
				if (posk >= 0 && pos >= 0 && posk <= row->width * row->height && pos <= frameg->width * frameg->height)
				{
					row->data[posk] = frameg->data[pos];
				}
			}
			yk++;
		}
		int b = 0;
		//vc_write_image((char*)"row.pgm", row);
		IVC* rb = vc_image_new(row->width, row->height, 1, 255);//Imagem para passar guardar a imagem em binario
		double media = 0, desvio = 0;
		CalculaMediaDesvio(row, &media, &desvio);//Calcular a media e o desvio padrão dos tons de cinzento na imagem
		vc_gray_to_binary_bernsen(row, rb, ceil(row->width * 0.02),media-desvio - 20);//Conversão de escala de cinzentos para binário pelo metodo de bernsen
		//em que o kernel é 2% da largura da imagem e o cmin é determinado pela media menos o desvio padrão menos 20;
		//vc_gray_to_binary(row, 200);
		//vc_write_image((char *)"rowb.pgm", rb);
		IVC *rc = vc_image_new(row->width, row->height, 1, 255);//Imagem para fazer a operação morfológica de abertura
		vc_binary_open(rb, rc, ceil(row->width * 0.005));//Operação morfológica de abertura com o kernel sendo 0.5% da largura da imagem
		//vc_write_image((char*)"rowc.pgm", rc);

		int nlabels = 0;//Variável para o numero de etiquetas
		IVC* labeled = vc_image_new(row->width, row->height, 1, 255);//Imagem para a etiquetagem
		OVC* blobs = vc_binary_blob_labelling(rc, labeled, &nlabels);//Etiquetagem da imagem que contém a matricula
		vc_binary_blob_info(labeled, blobs, nlabels);//Recolher a informação relativemente a cada etiqueta
		//vc_write_image((char*)"label.pgm",labeled);
		int blobIndex = -1;
		//Determinar qual dos blobs é a matricula
		for (int i = 0; i < nlabels; i++)
		{
			if (blobs[i].area > 0.1 * height * width && blobs[i].width/ blobs[i].height >= 4 && blobs[i].width / blobs[i].height <= 5)blobIndex = i;
		}

		int* hv = BinVHist(rc);//Histograma vertical		
		//for (int i = 0; i < width; i++)printf("%d\n", hv[i]);

		//Encontra o aonde a matricula começa no eixo horizontal
		int i = width / 2;
		while (i > 0)
		{
			if ((float)hv[i] / (float)height < 0.1)
			{
				xmin = i + 1;
				break;
			}
			i--;
		}
		//Encontra o aonde a matricula acaba no eixo horizontal
		i = (int)rc->width / 2;
		while(i<width)
		{
			if ((float)hv[i] / (float)height < 0.1) 
			{
				xmax = i + 1;
				break;
			}
			i++;
		}

		free(hv);
		//Compara-se os dados recolhidos para saber qual deles é o mais preciso 
		if (xmax - xmin >= blobs[blobIndex].width + 50)
		{
			xmin = blobs[blobIndex].x - 10;
			xmax = blobs[blobIndex].x + blobs[blobIndex].width + 10;
		}
		if (height >= blobs[blobIndex].height + 50)
		{
			ymin = blobs[blobIndex].y;
			ymax = blobs[blobIndex].y + blobs[blobIndex].height + 10;
			height = ymax - ymin;
		}

		//Desenho da caixa á volta da matricula
		//Desenho da linha horizontal superior da caixa delimitadora da matricula
		for (int x = xmin; x < xmax; x++)
		{
			pos = ymin * frame->bytesperline + x * frame->channels;
			if (pos > 0 && pos < frame->height * frame->bytesperline)
			{
				frame->data[pos] = (unsigned char)0;
				frame->data[pos + 1] = (unsigned char)255;
				frame->data[pos + 2] = (unsigned char)0;
			}
		}
		//Desenho da linha horizontal inferior da caixa delimitadora da matricula
		for (int x = xmin; x < xmax; x++)
		{
			pos = ymax * frame->bytesperline + x * frame->channels;
			if (pos > 0 && pos < frame->height * frame->bytesperline)
			{
				frame->data[pos] = (unsigned char)0;
				frame->data[pos + 1] = (unsigned char)255;
				frame->data[pos + 2] = (unsigned char)0;
			}
		}
		//Desenho da linha vertical esquerda da caixa delimitadora da matricula
		for (int y = ymin; y < ymax; y++)
		{
			pos = y * frame->bytesperline + xmin * frame->channels;
			if (pos > 0 && pos < frame->height * frame->bytesperline)
			{
				frame->data[pos] = (unsigned char)0;
				frame->data[pos + 1] = (unsigned char)255;
				frame->data[pos + 2] = (unsigned char)0;
			}
		}
		//Desenho da linha vertical direita da caixa delimitadora da matricula
		for (int y = ymin; y < ymax; y++)
		{
			pos = y * frame->bytesperline + xmax * frame->channels;
			if (pos > 0 && pos < frame->height * frame->bytesperline)
			{
				frame->data[pos] = (unsigned char)0;
				frame->data[pos + 1] = (unsigned char)255;
				frame->data[pos + 2] = (unsigned char)0;
			}
		}
		
		int width = xmax - xmin;//Largura da matricula
		IVC* matricula = vc_image_new(width, height, 1, 255);//Imagem para a matricula
		yk = 0;
		pos = 0;
		posk = 0;
		int xk = 0;
		//Copia a matricula para a imagem criada
		for (int y = ymin; y < ymax; y++)
		{
			for (int x = xmin; x < xmax; x++)
			{
				pos = y * frameg->width + x;
				posk = yk * width + xk;
				if (posk > 0 && posk < matricula->width * matricula->height && pos > 0 && pos < frameg->width * frameg->height);
				{
					matricula->data[posk] = frameg->data[pos];
				}
				xk++;
			}
			yk++;
			xk = 0;
		}
		
		if (matricula != NULL)
		{
			//vc_write_image((char*)"matricula.pgm", matricula);
			IVC* mb = vc_image_new(matricula->width, matricula->height, 1, 255);//Imagem para a matricula em binário
			//vc_gray_edge_prewitt(matricula, mb, 0.8);
			vc_gray_to_binary_bernsen(matricula, mb, 5, 150);//Conversão de escala de cinzentos para binário
			//vc_write_image((char*)"mb.pgm", mb);		
			IVC* mc = vc_image_new(matricula->width, matricula->height, 1, 255);
			vc_binary_open(mb, mc, 3);//Operação morfológica de abertura
			//vc_write_image((char*)"mo.pgm", mc);
			vc_bin_negative(mc);//Negativo da matricula
			//vc_write_image((char*)"mn.pgm", mc);
			
			//Variável para o número de etiquetas encontradas na etiquetagem da matricula 
			int nletras = 0;
			//Array para as etiquetas e a sua informação
			OVC* letras = vc_binary_blob_labelling(mc, matricula, &nletras);//Etiquetagem da imagem da matricula
			vc_binary_blob_info(matricula, letras, nletras);//Preenchimento do array com informação das etiquetas na imagen

			//vc_write_image((char*)"ml.pgm", matricula);

			IVC* caracter[6];//array de imagens para os caracteres da matricula
			for (int i = 0; i < 6; i++)caracter[i] = NULL;//Inicializar tudo a nulo

			float r = 0;
			int k = 0;
			int areatotal = matricula->height * matricula->width;
			//Percorrer o array de blobs e encontrar as letras
			for (int j = 0; j < nletras; j++)
			{
				r = (float)letras[j].width / (float)letras[j].height;
				if (r >= 0.4 && r < 1 && letras[j].area > areatotal * 0.007)//Condições que determinam o que é um caracter
				{
					//Desenho das caixas delimitadoras á volta da letras
					//Desenho da linha horizontal superior da caixa delimitadora da matricula
					for (int x = xmin + letras[j].x; x < xmin + letras[j].x + letras[j].width; x++)
					{
						pos = (ymin + letras[j].y) * frame->bytesperline + x * frame->channels;
						if (pos > 0 && pos < frame->height * frame->bytesperline)
						{
							frame->data[pos] = (unsigned char)0;
							frame->data[pos + 1] = (unsigned char)255;
							frame->data[pos + 2] = (unsigned char)0;
						}
					}
					//Desenho da linha horizontal inferior da caixa delimitadora da matricula
					for (int x = xmin + letras[j].x; x < xmin + letras[j].x + letras[j].width; x++)
					{
						pos = (ymin + letras[j].y + letras[j].height) * frame->bytesperline + x * frame->channels;
						if (pos > 0 && pos < frame->height * frame->bytesperline)
						{
							frame->data[pos] = (unsigned char)0;
							frame->data[pos + 1] = (unsigned char)255;
							frame->data[pos + 2] = (unsigned char)0;
						}
					}
					//Desenho da linha vertical esquerda da caixa delimitadora da matricula
					for (int y = ymin + letras[j].y; y < ymin + letras[j].y + letras[j].height; y++)
					{
						pos = y * frame->bytesperline + (xmin + letras[j].x) * frame->channels;
						if (pos > 0 && pos < frame->height * frame->bytesperline)
						{
							frame->data[pos] = (unsigned char)0;
							frame->data[pos + 1] = (unsigned char)255;
							frame->data[pos + 2] = (unsigned char)0;
						}
					}
					//Desenho da linha vertical direita da caixa delimitadora da matricula
					for (int y = ymin + letras[j].y; y < ymin + letras[j].y + letras[j].height; y++)
					{
						pos = y * frame->bytesperline + (xmin + letras[j].x + letras[j].width) * frame->channels;
						if (pos > 0 && pos < frame->height * frame->bytesperline)
						{
							frame->data[pos] = (unsigned char)0;
							frame->data[pos + 1] = (unsigned char)255;
							frame->data[pos + 2] = (unsigned char)0;
						}
					}
					//Criar uma imagem nova no array para o caracter encontrado 
					caracter[k] = vc_image_new(letras[j].width + 2, letras[j].height + 2, 1, 255);
					posk = 0;
					pos = 0;
					xk = 0;
					yk = 0;
					//copiar o caracter para essa imagem
					for (int y = letras[j].y - 1; y < letras[j].y + letras[j].height + 1; y++)
					{
						for (int x = letras[j].x - 1; x < letras[j].x + letras[j].width + 1; x++)
						{
							pos = y * mc->width + x;
							posk = yk * (letras[j].width + 2) + xk;
							caracter[k]->data[posk] = mc->data[pos];
							xk++;
						}
						yk++;
						xk = 0;
					}					
					//vc_write_image((char*)"caracter.pgm" , caracter[k]);
					k++;
				}
			}
			//identificação dos 6 caracteres
			for (int i = 0; i < 6; i++)
			{
				//vc_write_image((char*)"caracter.pgm", caracter[i]);
				c[i] = IdentificaCaracter(caracter[i]);
			}

			//for (int i = 0; i < 6; i++)
			//{
			//	printf("%c\n", c[i]);
			//}

			free(c);
			free(letras);
			free(mb);
			free(mc);
		}		
		free(matricula);		
		free(rc);
		free(rb);
		free(row);		
	}
	free(frameg);
}

//Histograma horizontal de uma imagem binaria
double* HistogramaHorizontalF(IVC* image)
{
	double* h = (double*)calloc(image->height, sizeof(double));
	int pos = 0;
	for (int y = 0; y < image->height; y++)
	{

		for (int x = 0; x < image->width; x++)
		{
			pos = y * image->bytesperline + x;

			if (image->data[pos] == (unsigned char)255)h[y] = h[y] + 1;
		}
	}
	return h;
}

//Histograma vertical de uma imagem binaria
double* HistogramaVerticalF(IVC* image)
{
	double* v = (double*)calloc(image->width, sizeof(double));
	int pos = 0;
	for (int x = 0; x < image->width; x++)
	{
		for (int y = 0; y < image->height; y++)
		{
			pos = y * image->width + x;

			if (image->data[pos] == (unsigned char)255)v[x] = v[x] + 1;
		}
	}
	return v;
}

//Função para identificar um caracter apartir de uma imagem
char IdentificaCaracter(IVC* image)
{
	if (image != NULL)
	{
		IVC* open = vc_image_new(image->width, image->height, 1, 255);
		vc_binary_open(image, open, 3);//Operação morfológica de abertura no caracter

		//Calculo da posição das linhas horizontais que contam as mudanças de cor
		int y1 = open->height / 4;
		int y2 = open->height * 2/4;
		int y3 = open->height * 3/4;
		int pos = 0;
		int linha1 = 0;
		unsigned char cor = 0;
		//Conta as mudanças de cor numa linha horizontal
		for (int x = 0; x < open->width;x++)
		{
			pos = y1 * open->width + x;
			if (open->data[pos] != cor)
			{
				linha1++;
				cor = open->data[pos];
			}

		}
		int linha2 = 0;
		cor = 0;
		//Conta as mudanças de cor numa linha horizontal
		for (int x = 0; x < open->width; x++)
		{
			pos = y2 * open->width + x;
			if (open->data[pos] != cor)
			{
				linha2++;
				cor = open->data[pos];
			}

		}
		int linha3 = 0;
		cor = 0;
		//Conta as mudanças de cor numa linha horizontal
		for (int x = 0; x < open->width; x++)
		{
			pos = y3 * open->width + x;
			if (open->data[pos] != cor)
			{
				linha3++;
				cor = open->data[pos];
			}

		}
		//Posição da linha vertical que conta as mudanças de cor
		int x1 = open->width / 2;
		int coluna1 = 0;
		cor = 0;;
		//Conta as mudanças de cor numa linha vertical
		for (int y = 0; y < open->height; y++)
		{
			pos = y * open->width + x1;
			if (open->data[pos] != cor)
			{
				coluna1++;
				cor = open->data[pos];
			}

		}

		//printf("y1=%d\n", linha1);
		//printf("y2=%d\n", linha2);
		//printf("y3=%d\n", linha3);
		//printf("x1=%d\n", coluna1);

		//vc_write_image((char*)"caractero.pgm", open);
		//Histograma vertical e horizontal da imagem
		double* histVertical = HistogramaVerticalF(open);
		double* histHorizontal = HistogramaHorizontalF(open);
		char caracter;
		//Aplicar a media movel aos histogramas
		mediaMovel(histHorizontal, image->height);
		mediaMovel(histVertical, image->width);
		//Aplicar a normalização aos histogramas
		normalizaHist(histHorizontal, image->height, image->width);
		normalizaHist(histVertical, image->width, image->height);
		int npicosH = 0;//numero de picos no histograma horizontal
		int npicosV = 0;//numero de picos no histograma vertical
		int* picosH = IndexPicos(histHorizontal, image->height, &npicosH);//array com o os indices dos picos horizontais
		int* picosV = IndexPicos(histVertical, image->width, &npicosV);//array com o os indices dos picos verticais

		//printf("%d\n", npicosH);
		//printf("%d\n", npicosV);

		//for (int i = 0; i < npicosH; i++)printf("Index: %d --> %f\n", picosH[i], histHorizontal[picosH[i]]);
		//for (int i = 0; i < npicosV; i++)printf("Index: %d --> %f\n", picosV[i], histVertical[picosV[i]]);

		//Reconehcimento da letra recebida através do numero de picos no histograma horizontal e vertical e pela posição relativa desses picos na imagem
		if (npicosH == 1 && (npicosV == 1 || npicosV == 2) && linha1 == 2 && linha2 == 2 && linha3 == 2 && (coluna1 == 4 || coluna1 == 3))return '7';
		if (npicosH == 3 && (npicosV == 3 || npicosV == 2)  && linha1 == 4 && linha2 == 4 && linha3 == 4 && coluna1 == 6)return 'Q';
		if ((npicosH == 1 || npicosH == 2) && npicosV == 2 && (linha1 == 3 || linha1 == 4) && (linha2 == 4 ||linha2 == 3) && linha3 == 4 && (coluna1 == 1 || coluna1 == 2))return 'U';
		if (npicosH == 2 && (npicosV == 2 || npicosV == 3) && picosV[0] < image->width / 3 && (linha1 == 4 || linha1 ==3) && linha2 == 2 && linha3 == 4 && (coluna1 == 4 || coluna1 == 5))return 'R';
		if ((npicosH == 3 || npicosH == 5) && npicosV == 2 && linha1 == 3 && (linha2 == 2 || linha2 == 1) && (linha3 == 4 || linha3 == 5) && (coluna1 == 5 || coluna1 == 6) && histVertical[picosV[0]] < histVertical[picosV[1]])return '9';
		if ((npicosH == 3 || npicosH == 4) && (npicosV == 3 || npicosV == 2) && (linha1 == 4 || linha1 == 3) && (linha2 == 2) && linha3 == 4 && coluna1 == 5 && histVertical[picosV[0] > histVertical[picosV[1]]])return '6';
		if ((npicosH == 3 || npicosH == 4 || npicosH == 5 || npicosH == 6) && npicosV == 2 && (linha1 == 3 || linha1 == 4) && linha2 == 2 && (linha3 == 4 || linha3 == 2) && (coluna1 == 6 || coluna1 == 5))return '8';
		if (npicosH == 2 && (npicosV == 1 || npicosV == 2 ||npicosV == 3) && linha1 == 3 && (linha2 == 2 || linha2 == 4) && linha3 == 2 && (coluna1 == 6 || coluna1 == 5))return '2';
		
		free(open);
		free(histHorizontal);                      
		free(histVertical);
		free(picosH);
		free(picosV); 

	}
}


//Faz a media de um histograma
float Media(double* hist, int n)
{
	int soma = 0;
	for (int i = 0; i < n; i++)
	{
		soma = soma + hist[i];
	}
	return soma / n;
}

//Função para encontrar o indice dos picos de um histograma
int* IndexPicos(double* histograma, int n, int *npicos)
{
	int picos[30];
	for (int i = 0; i < 30; i++)picos[i] = 0;
	int j = 0;
	int k = 0;
	while(j < n - 1)
	{
		if (histograma[j] < histograma[j + 1])
		{
			while (histograma[j] < histograma[j + 1])
			{
				j++;
			}
			if (histograma[j] >= 0.4)
			{
				picos[k] = j;
				k++;
			}
		}
		else
		{
			j++;
		}
	}

	int* p = (int*)calloc(k, sizeof(int));
	for (int i = 0; i < k; i++)
	{
		p[i] = picos[i];
	}
	*npicos = k;
	return p;
}

//Normaliza o histograma
void normalizaHist(double* histograma, int n, int totalPixeis)
{
	if (totalPixeis > 0)
	{
		for (int i = 0; i < n; i++)
		{
			histograma[i] = (histograma[i] / totalPixeis);
		}
	}
}

//Aplica a média movel a um histograma
void mediaMovel(double* histograma, int n)
{
	float soma = 0;
	for (int i = 0; i < n; i++)
	{
		for (int j = i - 2; j <= i + 2; j++)
		{
			if(j < n && j >= 0) soma = soma + histograma[j];
		}
		histograma[i] = soma / 5;
		soma = 0;
	}
}

//Histograma horizontal de uma imagem em escala de cinzentos
int* GetColunmProf(IVC* image)
{
	int x, y, soma, pos;
	int* h = (int *)malloc(sizeof(int) * image->width);

	for (x = 0; x < image->width; x++)
	{
		soma = 0;

		for (y = 0; y < image->height; y++)
		{
			pos = y * image->bytesperline + x;

			if (image->data[pos] == 0)soma++;
		}
		h[x] = soma;
	}
	return h;
}

//Histograma vertical de uma imagem em escala de cinzentos
int *GetRowProf(IVC* image, int y)
{
	int pos = 0;
	int *h = (int*)malloc(sizeof(int) * image->width);
	
	for (int x = 0; x < image->width; x++)
	{
		pos = y * image->bytesperline + x * image->channels;
		if (pos > 0 && pos < image->height * image->width)
		{
			h[x] = (int)image->data[pos];
			
		}
	}
	return h;
}

//Função para encontrar o numero de zonas de contraste de um histograma dividindo-o em várias partes iguais, encontrado o maximo e minimo em cada
//uma dessas partes e fazendo a diferença entre eles e indicando atrvés do booleano a se esses picos de contraste estão próximos uns dos outros
int vhistogram(int h[], int n, int kernel, int cmin,bool *a)
{
	int max, min;
	int v = 0;
	*a = false;
	int indexh[100];
	int m = 0;
	for (int i = 0; i < n; i = i + kernel)		
	{
		max = 0;
		min = 255;
		int c = i;
		max = h[c];
		for (c = i + 1; c < i + kernel; c++) {
			if (h[c] > max) {
				max = h[c];
			}
		}
		c = i;
		min = h[c];
		for (c = i + 1; c < i + kernel; c++) {
			if (h[c] < min) {
				min = h[c];
			}
		}

		if (max - min > cmin)
		{
			v++;
			indexh[m] = i;
			m++;
		}
	}
	int i = 0;
	int c = 0;
	while (i < m)
	{
		if (indexh[i + 1] - indexh[i] <= 2 * kernel)
		{
			c++;
			i++;
		}
		else
		{
			i++;
			c = 0;
		}
		if (c >= 6) *a = true;
	}

	return v;
}

//Calcula a media e o desvio padrão de uma imagem em escala de cinzentos
int CalculaMediaDesvio(IVC* image, double* media, double* desvio)
{
	
	int width = image->width;
	int heigth = image->height;
	if (width < 0 || heigth < 0 || media == NULL || desvio == NULL)return 0;
	double soma = 0;
	double sd = 0;
	*media = 0;
	*desvio = 0;

	for (int i = 0; i < width * heigth; i++)
	{
		soma = soma + (double)image->data[i];
	}
	*media = (soma / ((double)width * (double)heigth));	
	for (int i = 0; i < width * heigth; i++)
	{
		sd = sd + pow((double)image->data[i] - *media, 2);
	}
	*desvio = sqrt(sd / ((double)width * (double)heigth));

	return 1;
}


//Algorimo do trabalho prático 1
int BoundingBox(IVC* src)
{
	int pos = 0;
	IVC* gray = vc_image_new(src->width, src->height, 1, 255);//Imagem em cinzento para converter a imagem para cinzento
	IVC* bin = vc_image_new(src->width, src->height, 1, 255);//Imagen em binario para converter a imagem para binario
	IVC* close = vc_image_new(src->width, src->height, 1, 255);//Imagem em bin para fazer um fecho close 
	IVC* labeled = vc_image_new(src->width, src->height, 1, 255);//Imagem para os labels

	//A imagem é convertida para escala de cinzentos e depois para binário para ser mais de trabalhar com a imagem
	vc_rgb_to_gray(src, gray);//Converter a imagem original para Escala de cinzentos
	//vc_write_image("gray.pgm", gray);
	vc_gray_to_binary_bernsen(gray, bin, 5, 100);//Converter para binário usando o metodo de bernsen com um cmin de 240 para encontrar zonas de alto											 //contraste na imagem
	vc_binary_close(bin, close, 5);//Fazer um fecho da imagem com kernerl de 5
	//vc_write_image("bin.pgm", bin);
	//vc_write_image("close.pgm", close);	
	int labels = 0;//Variável para o número de labels encontrados
	OVC* blobs = vc_binary_blob_labelling(close, labeled, &labels);//Fazer o labelling da imagem para uma variável que vai conter a informação de todos																//os blobs na imagem, 1 deles sendo a matricula
	//vc_write_image("labeled.pgm", labeled);
	//printf("Labels: %d\n", labels);
	vc_binary_blob_info(labeled, blobs, labels);//Preencher o array de blobs com a sua informação(área,perimetro,largura,altura,Xinicial,Yinicial)
	//Filtragem dos blobls encontrados
	for (int i = 0; i < labels; i++)//Percorrer o array de blobs
	{
		if (blobs[i].height != 0 && blobs[i].width != 0 && blobs[i].area != 0)//Filtrar os blobs com valores inválidos
		{
			float r = blobs[i].width / blobs[i].height;//Calculo do racio largura/altura
			if (r >= 4 && r <= 5 && blobs[i].area > 2300 && blobs[i].area < 25000 && blobs[i].x != 1)//Se o racio estiver dentro dde 4 e 5 e a area for maior que 5000
			{
				
				printf("_______________BLOB________________\n");
				printf("Label: %d\n", blobs[i].label);
				printf("Width: %d\n", blobs[i].width);
				printf("Height: %d\n", blobs[i].height);
				printf("Area: %d\n", blobs[i].area);
				printf("X: %d\n", blobs[i].x);
				printf("Y: %d\n", blobs[i].y);

				//Criar uma imagem nova do tamanho do blob e copia-lo para lá
				IVC* blob = vc_image_new(blobs[i].width, blobs[i].height, 1, 255);//Imagem nova para analisar o blob e determinar se pode ser
																				//uma matricula ou não, com as dimensões do blob
				int bwidth = blobs[i].width;
				int bheight = blobs[i].height;

				int pos = 0;//Posição na imagem original
				int posb = 0;//Posição na imagem para o blob
				int xb = 0, yb = 0;
				//Copiar o blob encontrado, da imagem original para um imagem nova para ser analisado
				for (int y = blobs[i].y; y < bheight + blobs[i].y; y++)
				{
					for (int x = blobs[i].x; x < bwidth + blobs[i].x; x++)
					{
						pos = y * close->width + x;//Posição na imagem original
						posb = yb * bwidth + xb;//Posição na imagem do blob
						blob->data[posb] = gray->data[pos];//Copiar os dados
						xb++;//Incrementar uma coluna á posição no blob
					}
					yb++;//Incrementar uma linha á posição no blob
					xb = 0;//Por a posição nas colunas do blob a zero
				}

				//Desenho do limite superior do blob 
				for (int x = blobs[i].x; x < blobs[i].x + blobs[i].width; x++)
				{
					pos = blobs[i].y * src->bytesperline + x * src->channels;//Calculo da posiçao
					src->data[pos] = 0;//R
					src->data[pos + 1] = 255;//G
					src->data[pos + 2] = 255;//B
				}
				//Desenho do limite inferior do blob 
				for (int x = blobs[i].x; x < blobs[i].x + blobs[i].width; x++)
				{
					pos = (blobs[i].y + blobs[i].height) * src->bytesperline + x * src->channels;//Calculo da posiçao
					src->data[pos] = 0;//R
					src->data[pos + 1] = 255;//G
					src->data[pos + 2] = 255;//B
				}
				//Desenho do limite esquerdo do blob 
				for (int y = blobs[i].y; y < blobs[i].y + blobs[i].height; y++)
				{
					pos = y * src->bytesperline + blobs[i].x * src->channels;//Calculo da posiçao
					src->data[pos] = 0;//R
					src->data[pos + 1] = 255;//G
					src->data[pos + 2] = 255;//B
				}
				//Desenho do limite direito do blob 
				for (int y = blobs[i].y; y < blobs[i].y + blobs[i].height; y++)
				{
					pos = y * src->bytesperline + (blobs[i].x + blobs[i].width) * src->channels;//Calculo da posiçao
					src->data[pos] = 0;//R
					src->data[pos + 1] = 255;//G
					src->data[pos + 2] = 255;//B
				}
			#pragma region MyRegion

				//vc_write_image("blob.pgm", blob);

				IVC* matriculabin = vc_image_new(blob->width, blob->height, 1, 255);//Imagem para a imagem binaria do blob
				//IVC* matriculaclose = vc_image_new(blob->width, blob->height, 1, 255);//Imagem para o fecho
				IVC* matriculalabel = vc_image_new(blob->width, blob->height, 1, 255);//Imagem para os labels

				vc_gray_to_binary_bernsen(blob, matriculabin, 5, 240);//Conversão para binario com o metodo de bernsen
				//vc_write_image("matriculabin.pgm", matriculabin);
				vc_binary_close(matriculabin, blob, 5);//Operação morfológica de fecho com kerner 3x3				
				//vc_write_image("matriculaclose.pgm", matriculaclose);
				vc_bin_negative(blob);//Negativo do close
				int nletras = 0;//Variável para o numero de blobs
				//vc_write_image("matriculaclose.pgm", matriculaclose);
				OVC* letras = vc_binary_blob_labelling(blob, matriculalabel, &nletras);//Fazer o labelling da imagem
				//vc_write_image("matriculalabel.pgm", matriculalabel);

				vc_binary_blob_info(matriculalabel, letras, nletras);//Preencher o array com a informação de cada label
				int w = 0;
				//Verificar quais sao os blobs validos
				if (nletras >= 6)//Se o numero de blobs encontrados for menor 6 é descartado
				{

					for (int l = 0; l < nletras; l++)//Percorrer os array de labels
					{
						r = (float)letras[l].width / (float)letras[l].height;//Racio de largura/altura
						if (r > 0.25 && r < 0.9 && letras[l].area > 130)
						{
							w++;
							/*
							O desenho das letras é o mesmo processo da matricula sendo simplesmente com as dimensões das letras
							*/
							//Desenho do limite superior do blob 
							for (int x = letras[l].x + blobs[i].x; x < letras[l].x + letras[l].width + blobs[i].x; x++)
							{
								pos = (letras[l].y + blobs[i].y) * src->bytesperline + x * src->channels;//Calculo da posiçao
								src->data[pos] = 255;//R
								src->data[pos + 1] = 0;//G
								src->data[pos + 2] = 0;//B
							}
							//Desenho do limite inferior do blob 
							for (int x = letras[l].x + blobs[i].x; x < letras[l].x + letras[l].width + blobs[i].x; x++)
							{
								pos = (letras[l].y + letras[l].height + blobs[i].y) * src->bytesperline + x * src->channels;//Calculo da posiçao
								src->data[pos] = 255;//R
								src->data[pos + 1] = 0;//G
								src->data[pos + 2] = 0;//B
							}
							//Desenho do limite esquerdo do blob 
							for (int y = letras[l].y + blobs[i].y; y < letras[l].y + letras[l].height + blobs[i].y; y++)
							{
								pos = y * src->bytesperline + (blobs[i].x + letras[l].x) * src->channels;//Calculo da posiçao
								src->data[pos] = 255;//R
								src->data[pos + 1] = 0;//G
								src->data[pos + 2] = 0;//B
							}
							//Desenho do limite direito do blob 
							for (int y = letras[l].y + blobs[i].y; y < letras[l].y + letras[l].height + blobs[i].y; y++)
							{
								pos = y * src->bytesperline + (letras[l].x + letras[l].width + blobs[i].x) * src->channels;//Calculo da posiçao
								src->data[pos] = 255;//R
								src->data[pos + 1] = 0;//G
								src->data[pos + 2] = 0;//B
							}
						}
					}

					if (w == 6)
					{
						/*
					O desenho da bounding box da matricula é feita por partes, fazendo primeiro o desenho da reta superior percorrendo toda a
					largura da matricula na mesma linha, depois a reta inferior da bounding box pelo mesmo processom. O desenho das laterais
					da bounding box é feito percorrendo toda a altura da matricula numa coluna fixa.
					*/
					//Desenho do limite superior do blob 
						for (int x = blobs[i].x; x < blobs[i].x + blobs[i].width; x++)
						{
							pos = blobs[i].y * src->bytesperline + x * src->channels;//Calculo da posiçao
							src->data[pos] = 0;//R
							src->data[pos + 1] = 255;//G
							src->data[pos + 2] = 255;//B
						}
						//Desenho do limite inferior do blob 
						for (int x = blobs[i].x; x < blobs[i].x + blobs[i].width; x++)
						{
							pos = (blobs[i].y + blobs[i].height) * src->bytesperline + x * src->channels;//Calculo da posiçao
							src->data[pos] = 0;//R
							src->data[pos + 1] = 255;//G
							src->data[pos + 2] = 255;//B
						}
						//Desenho do limite esquerdo do blob 
						for (int y = blobs[i].y; y < blobs[i].y + blobs[i].height; y++)
						{
							pos = y * src->bytesperline + blobs[i].x * src->channels;//Calculo da posiçao
							src->data[pos] = 0;//R
							src->data[pos + 1] = 255;//G
							src->data[pos + 2] = 255;//B
						}
						//Desenho do limite direito do blob 
						for (int y = blobs[i].y; y < blobs[i].y + blobs[i].height; y++)
						{
							pos = y * src->bytesperline + (blobs[i].x + blobs[i].width) * src->channels;//Calculo da posiçao
							src->data[pos] = 0;//R
							src->data[pos + 1] = 255;//G
							src->data[pos + 2] = 255;//B
						}
					}
					w = 0;
				}
				free(letras);//Libertar a memoria usada para a estrutura que guarda a informação das letras
#pragma endregion
			}
		}
	}
	free(blobs);//Libertar a memoria usada para a estrutura que guarda a informação das matriculas
	free(bin);
	free(gray);
	free(labeled);
	return 1;
}
