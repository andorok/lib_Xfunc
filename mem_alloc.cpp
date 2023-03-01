
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

void* allocate_info_file(const char *fname)
{
	FILE *fp = fopen(fname, "rb");
	if (NULL == fp)
	{
		printf("\n\tERROR: Can't open file '%s'\n\n", fname);
		return NULL;
	}
	fpos_t pos;
	//fseek(fp, -SIZE_8M, SEEK_END);
	fseek(fp, -SIZE_256M, SEEK_END);

	printf("Allocating memory...              \r");
	// Выделяем память
	void *virt = virtAlloc(SIZE_256M);
	if (!virt)
	{
		printf("error: can not alloc %d Mbytes\n", SIZE_256M / SIZE_1M);
		return NULL;
	}

	printf("Reading file...                   \r");
	size_t readsize = SIZE_256M;
	size_t ret = fread(virt, 1, readsize, fp);
	if (ret != readsize)
	{
		printf("error: can not read %s\n", fname);
		return NULL;
	}

	fgetpos(fp, &pos);
	//fseek(fp, 0, SEEK_SET);
	fclose(fp);

	return virt;
}

#endif

const uint16_t g_sample_size = sizeof(uint16_t); // размер отсчета в байтах
const int g_num_chan = 16; // число каналов ЦАП
static vector<hupa_t> g_pages;

void* alloc_h2C_buffers(CXdmaDevice *pDev, const char* fname, int page_num, uint16_t chan)
{
	int status = 0;
	void* buffers = NULL;
	unsigned int blk_size = SIZE_128M;
	unsigned int blk_num = 1;
	uint16_t dir = DIR_H2C;
	char hpfname[128];
	//sprintf(hpfname, "%s_%03d", fname, 0);
	sprintf(hpfname, "%s%01d", fname, 0);

#ifdef __linux__
	unsigned int mem_type = PHYS_MEMORY_TYPE;
	unsigned int hupa_num = page_num;
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
		//sprintf(hpfname, "%s_%03d", fname, ipage);
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

	unsigned int hupa_num = page_num;
	void* virt = allocate_info_file(hpfname);

	uint32_t root_info_offset = *(uint32_t *)((uint8_t*)virt + ROOT_INFO_OFFSET_ADR);
	root_info_offset -= (SIZE_1G - SIZE_8M);
	root_info_t *ri = (root_info_t *)((uint8_t*)virt + root_info_offset);
	printf("root_info: MARKER %X, hp %d, blocks %d, nsmpl %d, data type %d, item width %d\n\n",
		ri->marker, ri->hp_curr, ri->block_count, ri->n_smpl, ri->data_type, ri->item_width);

	blk_num = ri->block_count * hupa_num;
	blk_size = ri->n_smpl * g_sample_size * g_num_chan;

	unsigned int mem_type = KERNEL_MEMORY_TYPE;
	status = pDev->alloc_dma_buf(&buffers, blk_size, &blk_num, mem_type, dir, chan);

#endif // __linux__
	//	status = pDev->alloc_dma_buf(&buffers, blk_size, &blk_num, mem_type, dir, chan);
	//status = pDev->alloc_dma_buf(&buffers, blk_size, &blk_num, mem_type, dir, chan, MODE_POLL);
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

		//printf(" H2C_%d Allocation buffer for DMA:  %d bytes", chan, (blk_size*blk_num));
		//switch (mem_type)
		//{
		//case USER_MEMORY_TYPE:
		//	printf(" USER MEMORY\n");
		//	break;
		//case KERNEL_MEMORY_TYPE:
		//	printf(" KERNEL MEMORY\n");
		//	break;
		//case PHYS_MEMORY_TYPE:
		//	printf(" PHYSICAL MEMORY\n");
		//	break;
		//}
#ifdef _WIN32
		int16_t **pBufferZ = (int16_t **)buffers;

		for (unsigned int ipage = 0; ipage < hupa_num; ipage++)
		{
			//sprintf(hpfname, "%s_%03d", fname, ipage);
			FILE *fp = fopen(hpfname, "rb");

			printf("Reading file %s ...                   \r", hpfname);
			for (unsigned int iBlock = 0; iBlock < blk_num; iBlock++)
			{
				fread(pBufferZ[iBlock], 1, blk_size, fp);
			}

			fclose(fp);
		}
		virtFree(virt);
#endif

		return buffers;
	}

}

void free_h2C_buffers(void *buf)
{
#ifdef __linux__
	for (auto &page : g_pages)
		free_hugepage(page);
	delete (void**)buf;
#endif
}
