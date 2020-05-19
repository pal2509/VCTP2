//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//           INSTITUTO POLITÉCNICO DO CÁVADO E DO AVE
//                          2011/2012
//             ENGENHARIA DE SISTEMAS INFORMÁTICOS
//                    VISÃO POR COMPUTADOR
//
//             [  DUARTE DUQUE - dduque@ipca.pt  ]
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++


#define VC_DEBUG


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//                   ESTRUTURA DE UMA IMAGEM
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++


typedef struct {
	unsigned char* data;
	int width, height;
	int channels;			// Binário/Cinzentos=1; RGB=3
	int levels;				// Binário=1; Cinzentos [1,255]; RGB [1,255]
	int bytesperline;		// width * channels
} IVC;


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//                    PROTÓTIPOS DE FUNÇÕES
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

// FUNÇÕES: ALOCAR E LIBERTAR UMA IMAGEM
IVC* vc_image_new(int width, int height, int channels, int levels);
IVC* vc_image_free(IVC* image);

// FUNÇÕES: LEITURA E ESCRITA DE IMAGENS (PBM, PGM E PPM)
IVC* vc_read_image(char* filename);
int vc_write_image(char* filename, IVC* image);

//FUNÇÕES:
int vc_gray_negative(IVC* srcdst);
int vc_rgb_negative(IVC* srcdst);
int vc_rgb_get_red_gray(IVC* srcdst);
int vc_rgb_get_green(IVC* srcdst);
int vc_rgb_get_blue_gray(IVC* srcdst);
int vc_rgb_to_gray(IVC* srcdst, IVC* gray);
int vc_rgb_to_hsv(IVC* srcdst);
int vc_hsv_segmentation(IVC* srcdst, int hmin, int hmax, int smin, int smax, int vmin, int vmax);
int vc_gray_scale_to_rgb(IVC* src, IVC* dst);
int vc_gray_to_binary(IVC* srcdst, int threshold);
int vc_bin_negative(IVC* srcdst);
int vc_gray_to_binary_global_mean(IVC* srcdst);
int vc_gray_to_binary_midpoint(IVC* src, IVC* dst, int kernel);
int vc_gray_to_binary_bersen(IVC* src, IVC* dst, int kernel, int cmin);
int vc_binary_dilate(IVC* src, IVC* dst, int kernel);
int vc_binary_erode(IVC* src, IVC* dst, int kernel);
int vc_binary_open(IVC* src, IVC* dst, int kernel);
int vc_binary_close(IVC* src, IVC* dst, int kernel); 
int vc_image_dupp(IVC* src, IVC* dst);
int vc_grayscale_dilate(IVC* src, IVC* dst, int kernel);
int vc_grayscale_erode(IVC* src, IVC* dst, int kernel);
int vc_grayscale_open(IVC* src, IVC* dst, int kernel);
int vc_grayscale_close(IVC* src, IVC* dst, int kernel);

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//                           MACROS
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
#define MAX(a, b) (a > b ? a : b)
#define MIN(a, b) (a < b ? a : b)


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//                   ESTRUTURA DE UM BLOB (OBJECTO)
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

typedef struct {
	int x, y, width, height;	// Caixa Delimitadora (Bounding Box)
	int area;					// Área
	int xc, yc;					// Centro-de-massa
	int perimeter;				// Perímetro
	int label;					// Etiqueta
} OVC;


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//                    PROTÓTIPOS DE FUNÇÕES
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

OVC* vc_binary_blob_labelling(IVC* src, IVC* dst, int* nlabels);
int vc_binary_blob_info(IVC* src, OVC* blobs, int nblobs);


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//                    FUNÇÕES DO TRABALHO PRÁTICO
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
int BoundingBox(IVC* src, IVC* dst);
int vc_rgb_to_gray_mean(IVC* src, IVC* dst);
int vc_gray_to_binary_bernsen(IVC* src, IVC* dst, int kernel, int cmin);