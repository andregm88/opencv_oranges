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


#define VC_DEBUG
#define MAX3(a,b,c) (a>b?(a>c?a:c):(b>c?b:c))
#define MIN3(a,b,c) (a<b?(a<c?a:c):(b<c?b:c))
#define ESPESSURA 3
#define RACIO 5.2
#define MARGIN 5


//////////////////////////////////////////////////////////////////
//                   ESTRUTURA DE UMA IMAGEM                    //
//////////////////////////////////////////////////////////////////

typedef struct {
	unsigned char *data;
	int width, height;
	int channels;			// Binario/Cinzentos=1; RGB=3
	int levels;				// Binario=1; Cinzentos [1,255]; RGB [1,255]
	int bytesperline;		// width * channels
} IVC;


//////////////////////////////////////////////////////////////////
//                   ESTRUTURA DE UM BLOB (OBJECTO)             //
//////////////////////////////////////////////////////////////////

typedef struct {
	int x, y, width, height;	// Caixa Delimitadora (Bounding Box)
	int area;					// �rea
	int xc, yc;					// Centro-de-massa
	int perimeter;				// Perimetro
	int label;					// Etiqueta

	unsigned char *mask;		// Not Used
	unsigned char *data;		// Not Used
	int channels;				// Not Used
	int levels;					// Not Used
} OVC;



//////////////////////////////////////////////////////////////////
//                           FUNÇÕES                            //
//////////////////////////////////////////////////////////////////

// alocar e libertar uma imagem
IVC *vc_image_new(int width, int height, int channels, int levels);
IVC *vc_image_free(IVC *image);

// leitura e escrita de imagens (pbm; pgm; ppm)
IVC *vc_read_image(char *filename);
int vc_write_image(char *filename, IVC *image);


// filtragem por cor
int vc_bgr_to_hsv_filter(IVC *srcdst);

// calcular minimo ou maximo
float vc_min_max(IVC *srcdst, float min_max, int mode);

// filtrar pelo min, media e max
int vc_min_max_filter(IVC *srcdst, float min, float max);

// passar para gray
int vc_img_to_gray(IVC *src, IVC *dst);

// passar para binario
int vc_gray_to_binary_global_mean(IVC *srcdst);

// passar para a frame
int vc_pix_to_frame(IVC *src, IVC *dst);

// trocar rgb <> bgr
int vc_change_rgb(IVC *srcdst);

// Etiquetagem de blobs
OVC* vc_binary_blob_labelling(IVC *src, IVC *dst, int *nlabels);

// info blobs
int vc_binary_blob_info(IVC *src, OVC *blobs, int nblobs);

// dilate
int vc_binary_dilate(IVC *src, IVC *dst, int kernel);