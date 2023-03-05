
#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>

#ifdef _WIN32
#include "windows.h"
#include <conio.h>
#endif

#include	"xdma_dev.h"
#include	"cfile.h"
#include	"hp_structs.h"

#define SIZE_4K 4 * 1024
#define SIZE_1M 1024 * 1024
#define SIZE_4M 4 * 1024 * 1024
#define SIZE_8M 8 * 1024 * 1024
#define SIZE_128M 128 * 1024 * 1024
#define SIZE_256M 256 * 1024 * 1024
#define SIZE_1G 1024*1024*1024

#ifdef __linux__

#include "pmem_common.h"
#define ADDR (void *) (0x0UL)   //starting address of the page

int allocate_hugepage(const char *fname, hupa_t *page)
{
	int pmem = open("/dev/pmem", O_RDWR);
	if (pmem == -1) {
		printf("error open /dev/pmem");
		return -1;
	}

	//page.fname = fname;
	page->fd = open(fname, O_CREAT | O_RDWR, S_IRWXU | S_IRWXG | S_IRWXO);
	if (page->fd < 0) {
		printf("error: can not open %s\n", fname);
		return -1;
	}

	if (fchmod(page->fd, S_IRWXU | S_IRWXG | S_IRWXO) != 0) {
		printf("error: chmod %s\n", fname);
		return -1;
	}

	//	page->virt = mmap(nullptr, hpage_size, PROT_READ | PROT_WRITE, MAP_SHARED | MAP_HUGETLB, page->fd, 0);
	page->virt = mmap(ADDR, SIZE_1G, PROT_READ | PROT_WRITE, MAP_SHARED, page->fd, 0);
	if (page->virt == MAP_FAILED) {
		printf("error: mmap %s, errno%d: %s\n", fname, errno, strerror(errno));
		return -1;
	}

	pmem_address_t a;
	a.virt = (unsigned long int)page->virt;
	a.phys = 0;
	ioctl(pmem, CONVERT_ADDRESS, &a);
	page->phys = (void*)a.phys;

	printf("page: virt 0x%02lX, phys 0x%02lX\n", (uint64_t)page->virt, (uint64_t)page->phys);

	close(pmem);

	return 0;
}

int free_hugepage(hupa_t page)
{
	munmap(page.virt, SIZE_1G);
	close(page.fd);
	return 0;
}
#else

// считать из файла информацию о данных
void* read_metainfo_from_file(const char *fname, size_t info_size)
{
	FILE *fp = fopen(fname, "rb");
	if (NULL == fp)
	{
		printf("\n\tERROR: Can't open file '%s'\n\n", fname);
		return NULL;
	}
	//fpos_t pos;
	//fseek(fp, -SIZE_8M, SEEK_END);
	fseek(fp, -(static_cast<long>(info_size)), SEEK_END);

	printf("Allocating memory...              \r");
	// Выделяем память
	void *virt = virtAlloc(info_size);
	if (!virt)
	{
		printf("error: can not alloc %zd Mbytes\n", info_size / SIZE_1M);
		return NULL;
	}

	printf("Reading file...                   \r");
	size_t readsize = info_size;
	size_t ret = fread(virt, 1, readsize, fp);
	if (ret != readsize)
	{
		printf("error: can not read %s\n", fname);
		return NULL;
	}

	//fgetpos(fp, &pos);
	//fseek(fp, 0, SEEK_SET);
	fclose(fp);

	return virt;
}

#endif

const uint16_t g_sample_size = sizeof(uint16_t); // размер отсчета в байтах
const int g_num_chan = 16; // число каналов ЦАП
static vector<hupa_t> g_pages;

// host2card (DAC) buffers
void* alloc_h2c_buffers(CXdmaDevice *pDev, const char* fname, int page_num, uint16_t chan)
{
	int status = 0;
	void* buffers = NULL;
	unsigned int blk_size = SIZE_128M;
	unsigned int blk_num = 1;
	uint16_t dir = DIR_H2C;
	char hpfname[128];
	//sprintf(hpfname, "%s_%03d", fname, 0);
	//sprintf(hpfname, "%s%01d", fname, 0);
	unsigned int hupa_num = page_num;

#ifdef __linux__
	unsigned int mem_type = PHYS_MEMORY_TYPE;
	//hupa_t page0;
	//if (allocate_hugepage(hpfname, &page0) < 0)
	//	return nullptr;
	//uint32_t root_info_offset = *(uint32_t *)((char *)page0.virt + ROOT_INFO_OFFSET_ADR);
	//root_info_t *ri = (root_info_t *)((char *)page0.virt + root_info_offset);
	//printf("root_info: MARKER %X, hp %d, blocks %d, nsmpl %d, data type %d, item width %d\n\n",
	//	ri->marker, ri->hp_curr, ri->block_count, ri->n_smpl, ri->data_type, ri->item_width);
	//blk_num = ri->block_count * hupa_num;
	//blk_size = ri->n_smpl * g_sample_size * g_num_chan;
	//free_hugepage(page0);

	//int blk_in_page = SIZE_1G / blk_size;
	//unsigned int hupa_num = blk_num / blk_in_page;
	//if (blk_num % blk_in_page) hupa_num++;
	root_info_t *ri = nullptr;
	void** data_buf = nullptr;
	for (unsigned int ipage = 0; ipage < hupa_num; ipage++)
	{
		hupa_t page;
		sprintf(hpfname, "%s%01d", fname, ipage);
		if (allocate_hugepage(hpfname, &page) < 0)
			return nullptr;
		g_pages.push_back(page);
		
		uint32_t root_info_offset = *(uint32_t *)((uint8_t*)page.virt + ROOT_INFO_OFFSET_ADR);
		ri = (root_info_t *)((char *)page.virt + root_info_offset);
		printf("root_info: MARKER %X, hp %d, blocks %d, nsmpl %d, data type %d, item width %d\n\n",
			ri->marker, ri->hp_curr, ri->block_count, ri->n_smpl, ri->data_type, ri->item_width);
		if(ipage == 0)
		{
			blk_num = ri->block_count * hupa_num;
			blk_size = ri->n_smpl * g_sample_size * g_num_chan;
			data_buf = new void*[blk_num];
		}
		//uint32_t cur_root_info_offset = *(uint32_t *)((char *)page.virt + ROOT_INFO_OFFSET_ADR);
		//root_info_t *cur_ri = (root_info_t *)((char *)page.virt + root_info_offset);

		//uint32_t offset;
		for (int iblk = 0; iblk < ri->block_count; iblk++)
		{
			uint32_t block_info_offset = ri->block_info_offset[iblk];
			block_info_t *bi = (block_info_t *)((char *)page.virt + block_info_offset);
			printf("block_info %d: MARKER %X, nsmpl %d, nframe %d, sec %d, nsec %d \n",
					iblk, bi->marker, bi->n_smpl, bi->nframe, bi->sec, bi->nsec);
			if(iblk == 0 && ipage == 0)
			{
				blk_size = bi->n_smpl * g_sample_size * g_num_chan;
			}

 			sig_offsets_t signal_offset = ri->sig_offsets[iblk];
			data_buf[ipage * ri->block_count + iblk] = (uint8_t*)page.phys + signal_offset.offset;
			printf(" page%d blk%d phys=0x%lX (offset=0x%08X)\n", ipage, iblk, (u_long)data_buf[ipage * ri->block_count + iblk], signal_offset.offset);
		}
	}
	status = pDev->alloc_dma_buf(data_buf, blk_size, &blk_num, mem_type, dir, chan);
	for (unsigned int ipage = 0; ipage < hupa_num; ipage++)
	{
		hupa_t page = g_pages[ipage];
		//uint32_t offset;
		for (unsigned int iblk = 0; iblk < blk_num; iblk++)
		{
			sig_offsets_t signal_offset = ri->sig_offsets[iblk];
			data_buf[ipage * ri->block_count + iblk] = (uint8_t*)page.virt + signal_offset.offset;
		}
	}
	buffers = data_buf;
	//data_buf[ipage] = (uint8_t*)page.virt;
	//buffers = data_buf;
	//blk_num = 8 * hupa_num;
	//status = pDev->alloc_dma_hupa(hpfname, &buffers, blk_size, hupa_num, &blk_num, dir, chan);
#else

	sprintf(hpfname, "%s%01d", fname, 0);
	size_t info_size = SIZE_256M;
	void* virt = read_metainfo_from_file(hpfname, info_size);
	uint32_t offset_correcting = SIZE_1G - static_cast<uint32_t>(info_size);

	uint32_t root_info_offset = *(uint32_t *)((uint8_t*)virt + (info_size-4));
	root_info_offset -= offset_correcting;
	root_info_t *ri = (root_info_t *)((uint8_t*)virt + root_info_offset);
	printf("root_info: MARKER %X, hp %d, blocks %d, nsmpl %d, data type %d, item width %d\n\n",
		ri->marker, ri->hp_curr, ri->block_count, ri->n_smpl, ri->data_type, ri->item_width);

	blk_num = ri->block_count * hupa_num;
	blk_size = ri->n_smpl * g_sample_size * g_num_chan;
	uint32_t* bi_offset = new uint32_t[blk_num];;

	for (unsigned int ipage = 0; ipage < hupa_num; ipage++)
	{
		for (int iblk = 0; iblk < ri->block_count; iblk++)
		{
			uint32_t block_info_offset = ri->block_info_offset[iblk];
			block_info_offset -= offset_correcting;
			block_info_t *bi = (block_info_t *)((char *)virt + block_info_offset);
			printf("block_info %d: MARKER %X, nsmpl %d, nframe %d, sec %d, nsec %d \n",
				iblk, bi->marker, bi->n_smpl, bi->nframe, bi->sec, bi->nsec);
			if (iblk == 0 && ipage == 0)
			{
				blk_size = bi->n_smpl * g_sample_size * g_num_chan;
			}
			sig_offsets_t signal_offset = ri->sig_offsets[iblk];
			bi_offset[ipage * ri->block_count + iblk] = signal_offset.offset;
			printf("  blk%d offset = 0x%08X ( 0x%08X )\n", iblk, bi_offset[ipage * ri->block_count + iblk], signal_offset.offset - offset_correcting);
		}
	}

	unsigned int mem_type = KERNEL_MEMORY_TYPE;
	status = pDev->alloc_dma_buf(&buffers, blk_size, &blk_num, mem_type, dir, chan);

#endif // __linux__

	if ((status < 0) || (blk_size < SIZE_4K))
	{
		printf(" C2H_%d ERROR by allocation buffer for DMA:  %d bytes \n", chan, blk_size);
		return NULL;
	}
	else
	{
		printf(" C2H_%d Allocation buffer for DMA:  %dkb (%dkb * %d) ", chan,
						(blk_size*blk_num) / 1024, blk_size / 1024, blk_num);
		if (mem_type == PHYS_MEMORY_TYPE) printf(" PHYSICAL MEMORY\n");
		else printf(" KERNEL MEMORY\n");

#ifdef _WIN32
		int16_t **pBufferZ = (int16_t **)buffers;

		for (unsigned int ipage = 0; ipage < hupa_num; ipage++)
		{
			sprintf(hpfname, "%s%01d", fname, ipage);
			FILE *fp = fopen(hpfname, "rb");

			printf("Reading file %s ...                   \r", hpfname);
			for (unsigned int iBlock = 0; iBlock < blk_num; iBlock++)
			{
				fseek(fp, bi_offset[ipage * ri->block_count + iBlock], SEEK_SET);
				fread(pBufferZ[iBlock], 1, blk_size, fp);
			}

			fclose(fp);
		}
		virtFree(virt);
#endif

		return buffers;
	}

}

void free_buffers(void *buf)
{
#ifdef __linux__
	for (auto &page : g_pages)
		free_hugepage(page);
	delete (void**)buf;
#endif
}

#define DATA_EMPTY      0   // данных нет
#define DATA_BUSY       1   // фрейм в процессе заполнения
#define DATA_PRESENT    2   // фрейм с данными
#define DATA_ERROR      3   // была ошибка при приеме фрейма
#define DATA_DONTUSE    4   // метка для не используемых фреймов, обычно на последней странице

#ifdef __linux__

// card2host (ADC) buffers
void* alloc_c2h_buffers(CXdmaDevice *pDev, const char* fname, int ndev, int chans, int samples, int page_num,
							uint32_t blk_size, uint32_t blk_num, uint16_t dma_chan)
{
	int status = 0;

	void* buffers = NULL;
	
	int hupa_num = page_num;
	int blk_in_page = SIZE_1G / blk_size - 1;

	uint32_t root_info_offset = SIZE_1G - (sizeof(root_info_t) + blk_in_page * sizeof(block_info_t));
	root_info_offset = 4096 * (root_info_offset / 4096);

	vector<hupa_t> pages;
	char hpfname[160];
	void** data_buf = new void*[hupa_num * blk_in_page];
	for (int ipage = 0; ipage < hupa_num; ipage++)
	{
		hupa_t page;
		sprintf(hpfname, "%s%02d_%03d", fname, pDev->get_instance(), ipage);
		if (allocate_hugepage(hpfname, &page) < 0)
			return nullptr;
		pages.push_back(page);
		uint32_t *ro = (uint32_t *)((char *)page.virt + ROOT_INFO_OFFSET_ADR);
		*ro = root_info_offset;

		root_info_t *ri = (root_info_t *)((char *)page.virt + root_info_offset);
		memset(ri, 0, sizeof(struct root_info_t));
		ri->marker = ROOT_MARKER;
		ri->bacos_total = ndev;				// всего устройств
		ri->bacos_curr = pDev->get_instance(); // номер устройства
		ri->hp_total = hupa_num;			// всего hugepage`й
		ri->hp_curr = ipage;				// номер hugepage
		ri->ach_total = chans * ndev;		// всего каналов
		ri->ach_start = 0;
		ri->ach_count = chans;				// каналов на одном устройстве
		ri->block_count = blk_in_page;		// количество блоков на hugepage
		ri->n_smpl = samples;				// количество отсчетов на канал в блоке (фрейме)
		ri->data_type = DATA_TYPE_SIG_TIME; // тип данных - времянной сигнал
		ri->n_lines_per_block = 1;
		ri->mips = -1;						// количество масштабов для DATA_TYPE_PANO_ReImDAQPw

		uint32_t blk_offset, bi_offset;
		for (int iblk = 0; iblk < blk_in_page; iblk++)
		{
			blk_offset = iblk * blk_size;
			bi_offset = root_info_offset + sizeof(root_info_t) + iblk * sizeof(block_info_t);
			int idx_blk = ipage * blk_in_page + iblk;
			ri->sig_offsets[iblk].offset = blk_offset;
			ri->block_info_offset[iblk] = bi_offset;
			data_buf[idx_blk] = (uint8_t*)page.phys + blk_offset;
			printf(" page%d blk%d phys=0x%lX\n", ipage, iblk, (u_long)data_buf[idx_blk]);

			block_info_t* blk_info = (block_info_t*)((uint8_t*)page.virt + bi_offset);
			blk_info->marker = BLOCK_MARKER;
			blk_info->nframe = 0;
			blk_info->sec = 0;
			blk_info->nsec = 0;
			blk_info->l_freq_Hz = 0;
			blk_info->bin_Hz = 0;
			blk_info->n_smpl = samples;
			blk_info->ach_total = ri->ach_total; // всего каналов
			blk_info->status = DATA_EMPTY;
			blk_info->hw_nframe = -1;
			blk_info->block_idx = idx_blk;
			blk_info->block_idx_dst = -1;
		}
	}

	unsigned int mem_type = PHYS_MEMORY_TYPE;
	uint16_t dir = DIR_C2H;
	status = pDev->alloc_dma_buf(data_buf, blk_size, &blk_num, mem_type, dir, dma_chan);
	
	for (int ipage = 0; ipage < hupa_num; ipage++)
	{
		hupa_t page = pages[ipage];
		uint32_t offset;
		for (int iblk = 0; iblk < blk_in_page; iblk++)
		{
			offset = iblk * blk_size;
			data_buf[ipage * blk_in_page + iblk] = (uint8_t*)page.virt + offset;
		}
	}
	buffers = data_buf;

	if ((status < 0) || (blk_size < SIZE_4K))
	{
		printf(" C2H_%d ERROR by allocation buffer for DMA:  %d bytes \n", dma_chan, blk_size);
		return nullptr;
	}
	else
	{
		printf(" C2H_%d Allocation buffer for DMA:  %dkb (%dkb * %d) ", dma_chan,
			(blk_size*blk_num) / 1024, blk_size / 1024, blk_num);
		if (mem_type == USER_MEMORY_TYPE) printf(" USER MEMORY\n");
		else printf(" KERNEL MEMORY\n");

		return buffers;
	}
}

#else

// card2host (ADC) buffers
void* alloc_c2h_buffers(CXdmaDevice *pDev, const char* fname, int ndev, int chans, int samples, int page_num,
	uint32_t blk_size, uint32_t blk_num, uint16_t dma_chan)
{
	int status = 0;

	void* buffers = NULL;

	//char hpfname[128];
	int hupa_num = page_num;

	int blk_in_page = SIZE_1G / blk_size - 1;

	//void** root_info_ptr = new void*[g_pages];
	struct root_info_t** root_info = new struct root_info_t*[hupa_num];

	for (int ipage = 0; ipage < hupa_num; ipage++)
	{
		//struct root_info_t* root_info = new root_info_t;
		//root_info_ptr[ipage] = root_info;
		root_info[ipage] = new root_info_t;

		memset(root_info[ipage], 0, sizeof(struct root_info_t));
		root_info[ipage]->marker = ROOT_MARKER;
		root_info[ipage]->bacos_total = ndev;
		root_info[ipage]->bacos_curr = pDev->get_instance();
		root_info[ipage]->hp_total = hupa_num;			// всего hugepage`й
		root_info[ipage]->hp_curr = ipage;				// номер hugepage
		root_info[ipage]->ach_total = chans * ndev; // всего каналов
		root_info[ipage]->ach_start = 0;
		root_info[ipage]->ach_count = chans;
		root_info[ipage]->block_count = blk_in_page;	// количество блоков на hugepage
		root_info[ipage]->n_smpl = samples;			// количество отсчетов в блоке (фрейме)
		root_info[ipage]->data_type = DATA_TYPE_SIG_TIME;
		root_info[ipage]->n_lines_per_block = 1;
		root_info[ipage]->mips = -1;            // количество масштабов для DATA_TYPE_PANO_ReImDAQPw
	}

	unsigned int mem_type = KERNEL_MEMORY_TYPE;
	uint16_t dir = DIR_C2H;

	block_info_t** blk_info = new block_info_t*[hupa_num * blk_in_page];

	for (int ipage = 0; ipage < hupa_num; ipage++)
	{
		//struct root_info_t* root_info = (struct root_info_t*)root_info_ptr[ipage];
		for (int iblk = 0; iblk < blk_in_page; iblk++)
		{
			root_info[ipage]->sig_offsets[iblk].offset = iblk * blk_size;
			//block_info_t *bi = (block_info_t *)virtAlloc(SIZE_4K);
			int idx_blk = ipage * blk_in_page + iblk;
			blk_info[idx_blk] = (block_info_t *)virtAlloc(SIZE_4K);
			root_info[ipage]->block_info_offset[iblk] = iblk * SIZE_4K;
			blk_info[idx_blk]->marker = BLOCK_MARKER;
			blk_info[idx_blk]->nframe = 0;
			blk_info[idx_blk]->sec = 0;
			blk_info[idx_blk]->nsec = 0;
			blk_info[idx_blk]->l_freq_Hz = 0;
			blk_info[idx_blk]->bin_Hz = 0;
			blk_info[idx_blk]->n_smpl = samples;
			blk_info[idx_blk]->ach_total = root_info[ipage]->ach_total; // всего каналов
			blk_info[idx_blk]->status = DATA_EMPTY;
			blk_info[idx_blk]->hw_nframe = -1;
			blk_info[idx_blk]->block_idx = idx_blk;
			blk_info[idx_blk]->block_idx_dst = -1;
			//info_buf[ipage * blk_in_page + iblk] = bi;
		}
	}
	status = pDev->alloc_dma_buf(&buffers, blk_size, &blk_num, mem_type, dir, dma_chan);

	if ((status < 0) || (blk_size < SIZE_4K))
	{
		printf(" C2H_%d ERROR by allocation buffer for DMA:  %d bytes \n", dma_chan, blk_size);
		return nullptr;
	}
	else
	{
		printf(" C2H_%d Allocation buffer for DMA:  %dkb (%dkb * %d) ", dma_chan,
			(blk_size*blk_num) / 1024, blk_size / 1024, blk_num);
		if (mem_type == USER_MEMORY_TYPE) printf(" USER MEMORY\n");
		else printf(" KERNEL MEMORY\n");

		return buffers;
	}
}
#endif
