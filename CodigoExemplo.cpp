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


		// Faça o seu código aqui...
		// Cria uma nova imagem IVC
		IVC* image = vc_image_new(video.width, video.height, 3, 255);

		// Copia dados de imagem da estrutura cv::Mat para uma estrutura IVC
		memcpy(image->data, frame.data, video.width * video.height * 3);
		// Executa uma função da nossa biblioteca vc
		//vc_rgb_get_green(image);
		BoundingBox(image);			

		// Copia dados de imagem da estrutura IVC para uma estrutura cv::Mat
		memcpy(frame.data, image->data, video.width * video.height * 3);
		// Liberta a memória da imagem IVC que havia sido criada
		vc_image_free(image);
		// +++++++++++++++++++++++++

		/* Exibe a frame */
		cv::imshow("VC - VIDEO", frame);

		/* Sai da aplicação, se o utilizador premir a tecla 'q' */
		key = cv::waitKey(1);
	}


	/* Para o timer e exibe o tempo decorrido */
	vc_timer();
	
	/* Fecha a janela */
	cv::destroyWindow("VC - VIDEO");

	/* Fecha o ficheiro de vídeo */
	capture.release();

	return 0;
}

int Frame(IVC* frame)
{
	
}

int IsPlate(IVC* blob)
{

}



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

				////Criar uma imagem nova do tamanho do blob e copia-lo para lá
				//IVC* blob = vc_image_new(blobs[i].width, blobs[i].height, 1, 255);//Imagem nova para analisar o blob e determinar se pode ser
				//																//uma matricula ou não, com as dimensões do blob
				//int bwidth = blobs[i].width;
				//int bheight = blobs[i].height;

				//int pos = 0;//Posição na imagem original
				//int posb = 0;//Posição na imagem para o blob
				//int xb = 0, yb = 0;
				////Copiar o blob encontrado, da imagem original para um imagem nova para ser analisado
				//for (int y = blobs[i].y; y < bheight + blobs[i].y; y++)
				//{
				//	for (int x = blobs[i].x; x < bwidth + blobs[i].x; x++)
				//	{
				//		pos = y * close->width + x;//Posição na imagem original
				//		posb = yb * bwidth + xb;//Posição na imagem do blob
				//		blob->data[posb] = gray->data[pos];//Copiar os dados
				//		xb++;//Incrementar uma coluna á posição no blob
				//	}
				//	yb++;//Incrementar uma linha á posição no blob
				//	xb = 0;//Por a posição nas colunas do blob a zero
				//}

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

				////vc_write_image("blob.pgm", blob);

				//IVC* matriculabin = vc_image_new(blob->width, blob->height, 1, 255);//Imagem para a imagem binaria do blob
				////IVC* matriculaclose = vc_image_new(blob->width, blob->height, 1, 255);//Imagem para o fecho
				//IVC* matriculalabel = vc_image_new(blob->width, blob->height, 1, 255);//Imagem para os labels

				//vc_gray_to_binary_bernsen(blob, matriculabin, 5, 240);//Conversão para binario com o metodo de bernsen
				////vc_write_image("matriculabin.pgm", matriculabin);
				//vc_binary_close(matriculabin, blob, 5);//Operação morfológica de fecho com kerner 3x3				
				////vc_write_image("matriculaclose.pgm", matriculaclose);
				//vc_bin_negative(blob);//Negativo do close
				//int nletras = 0;//Variável para o numero de blobs
				////vc_write_image("matriculaclose.pgm", matriculaclose);
				//OVC* letras = vc_binary_blob_labelling(blob, matriculalabel, &nletras);//Fazer o labelling da imagem
				////vc_write_image("matriculalabel.pgm", matriculalabel);

				//vc_binary_blob_info(matriculalabel, letras, nletras);//Preencher o array com a informação de cada label
				//int w = 0;
				////Verificar quais sao os blobs validos
				//if (nletras >= 6)//Se o numero de blobs encontrados for menor 6 é descartado
				//{

				//	for (int l = 0; l < nletras; l++)//Percorrer os array de labels
				//	{
				//		r = (float)letras[l].width / (float)letras[l].height;//Racio de largura/altura
				//		if (r > 0.25 && r < 0.9 && letras[l].area > 130)
				//		{
				//			w++;
				//			/*
				//			O desenho das letras é o mesmo processo da matricula sendo simplesmente com as dimensões das letras
				//			*/
				//			//Desenho do limite superior do blob 
				//			for (int x = letras[l].x + blobs[i].x; x < letras[l].x + letras[l].width + blobs[i].x; x++)
				//			{
				//				pos = (letras[l].y + blobs[i].y) * src->bytesperline + x * src->channels;//Calculo da posiçao
				//				src->data[pos] = 255;//R
				//				src->data[pos + 1] = 0;//G
				//				src->data[pos + 2] = 0;//B
				//			}
				//			//Desenho do limite inferior do blob 
				//			for (int x = letras[l].x + blobs[i].x; x < letras[l].x + letras[l].width + blobs[i].x; x++)
				//			{
				//				pos = (letras[l].y + letras[l].height + blobs[i].y) * src->bytesperline + x * src->channels;//Calculo da posiçao
				//				src->data[pos] = 255;//R
				//				src->data[pos + 1] = 0;//G
				//				src->data[pos + 2] = 0;//B
				//			}
				//			//Desenho do limite esquerdo do blob 
				//			for (int y = letras[l].y + blobs[i].y; y < letras[l].y + letras[l].height + blobs[i].y; y++)
				//			{
				//				pos = y * src->bytesperline + (blobs[i].x + letras[l].x) * src->channels;//Calculo da posiçao
				//				src->data[pos] = 255;//R
				//				src->data[pos + 1] = 0;//G
				//				src->data[pos + 2] = 0;//B
				//			}
				//			//Desenho do limite direito do blob 
				//			for (int y = letras[l].y + blobs[i].y; y < letras[l].y + letras[l].height + blobs[i].y; y++)
				//			{
				//				pos = y * src->bytesperline + (letras[l].x + letras[l].width + blobs[i].x) * src->channels;//Calculo da posiçao
				//				src->data[pos] = 255;//R
				//				src->data[pos + 1] = 0;//G
				//				src->data[pos + 2] = 0;//B
				//			}
				//		}
				//	}

				//	if (w == 6)
				//	{
				//		/*
				//	O desenho da bounding box da matricula é feita por partes, fazendo primeiro o desenho da reta superior percorrendo toda a
				//	largura da matricula na mesma linha, depois a reta inferior da bounding box pelo mesmo processom. O desenho das laterais
				//	da bounding box é feito percorrendo toda a altura da matricula numa coluna fixa.
				//	*/
				//	////Desenho do limite superior do blob 
				//	//	for (int x = blobs[i].x; x < blobs[i].x + blobs[i].width; x++)
				//	//	{
				//	//		pos = blobs[i].y * src->bytesperline + x * src->channels;//Calculo da posiçao
				//	//		src->data[pos] = 0;//R
				//	//		src->data[pos + 1] = 255;//G
				//	//		src->data[pos + 2] = 255;//B
				//	//	}
				//	//	//Desenho do limite inferior do blob 
				//	//	for (int x = blobs[i].x; x < blobs[i].x + blobs[i].width; x++)
				//	//	{
				//	//		pos = (blobs[i].y + blobs[i].height) * src->bytesperline + x * src->channels;//Calculo da posiçao
				//	//		src->data[pos] = 0;//R
				//	//		src->data[pos + 1] = 255;//G
				//	//		src->data[pos + 2] = 255;//B
				//	//	}
				//	//	//Desenho do limite esquerdo do blob 
				//	//	for (int y = blobs[i].y; y < blobs[i].y + blobs[i].height; y++)
				//	//	{
				//	//		pos = y * src->bytesperline + blobs[i].x * src->channels;//Calculo da posiçao
				//	//		src->data[pos] = 0;//R
				//	//		src->data[pos + 1] = 255;//G
				//	//		src->data[pos + 2] = 255;//B
				//	//	}
				//	//	//Desenho do limite direito do blob 
				//	//	for (int y = blobs[i].y; y < blobs[i].y + blobs[i].height; y++)
				//	//	{
				//	//		pos = y * src->bytesperline + (blobs[i].x + blobs[i].width) * src->channels;//Calculo da posiçao
				//	//		src->data[pos] = 0;//R
				//	//		src->data[pos + 1] = 255;//G
				//	//		src->data[pos + 2] = 255;//B
				//	//	}
				//	}
				//	w = 0;
				//}
				//free(letras);//Libertar a memoria usada para a estrutura que guarda a informação das letras
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
