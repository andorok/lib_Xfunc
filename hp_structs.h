#ifndef HP_STRUCTS_H
#define HP_STRUCTS_H

#include <cstdint>

//#ifdef __linux__
#define ROOT_INFO_OFFSET_ADR (1024 * 1024 * 1024 - 4)
//#else
//#define ROOT_INFO_OFFSET_ADR (256 * 1024 * 1024 - 4)
//#endif // __linux__

#define ROOT_MARKER 0xAAAABBBB
#define BLOCK_MARKER 0xCCCCDDDD

//#define MAX_BLOCKS_PER_HP 64
//#define MAX_BLOCKS_PER_HP 256
#define MAX_BLOCKS_PER_HP 512
#define MAX_AFS_CHANS 128
#define MAX_LINES_PER_BLOCK 1024

#define DATA_TYPE_SIG_TIME 2
// int16_t smpl[n_smpl][ch][2]

#define DATA_TYPE_SIG_FREQ_INTERLEAVED_INT32 3
// int32_t smpl[ch][2][n_smpl]

#define DATA_TYPE_SIG_FREQ_INTERLEAVED_FLOAT 4
// float smpl[ch][2][n_smpl]

#define DATA_TYPE_SIG_FREQ_PLANAR_INT32 5
// int32_t smpl[n_smpl][ch][2]

#define DATA_TYPE_SIG_FREQ_PLANAR_FLOAT 6
// float smpl[n_smpl][ch][2]

#define DATA_EMPTY      0   // данных нет
#define DATA_BUSY       1   // фрейм в процессе заполнения
#define DATA_PRESENT    2   // фрейм с данными
#define DATA_ERROR      3   // была ошибка при приеме фрейма
#define DATA_DONTUSE    4   // метка для не используемых фреймов, обычно на последней странице

#ifdef _WIN32
#pragma pack(push,1)
#endif

#ifdef __linux__
struct __attribute__((packed)) sig_offsets_t
#else
struct sig_offsets_t
#endif // __linux__
{
    int32_t offset;
};

// для доступа simple
#define DATA_TYPE_SIG_COMBINED_SIMPLE 20
#ifdef __linux__
struct __attribute__((packed)) combined_simple_offsets_t
#else
struct combined_simple_offsets_t
#endif // __linux__
{
    int32_t data;    // Отступ начала первой порции сжатых данных, -1 если не используется
    int32_t counter; // Отступ начала массива счётчиков полезного размера (в количестве int), -1 если не используется
    int32_t re; // float re[n_smpl], суммарный канал, -1 если не используется
    int32_t im; // float im[n_smpl], суммарный канал, -1 если не используется
    int32_t pw; // uint16_t pw[n_smpl], мощность сигнала для панорамы, -1 если не используется
    int32_t hash; // uint8_t hash[n_smpl], хэш быстрых пеленгов, -1 если не используется
    int32_t d; // uint8_t d[n_smpl], суммарный канал, -1 если не используется
    int32_t a; // uint8_t a[n_smpl], суммарный канал, -1 если не используется
    int32_t q; // uint8_t q[n_smpl], суммарный канал, -1 если не используется
};

// для доступа direct ?
#define DATA_TYPE_SIG_COMBINED_SCALED 21
#ifdef __linux__
struct __attribute__((packed)) combined_scaled_offsets_t
#else
struct combined_scaled_offsets_t
#endif // __linux__
{
    int32_t data[MAX_LINES_PER_BLOCK];    // Отступ начала первой порции сжатых данных
    int32_t counter[MAX_LINES_PER_BLOCK]; // Отступ начала массива счётчиков полезного размера (в количестве int)
    int32_t re[MAX_LINES_PER_BLOCK];
    int32_t im[MAX_LINES_PER_BLOCK];
    int32_t pw[MAX_LINES_PER_BLOCK];
    int32_t hash[MAX_LINES_PER_BLOCK];
    int32_t d[MAX_LINES_PER_BLOCK];
    int32_t a[MAX_LINES_PER_BLOCK];
    int32_t q[MAX_LINES_PER_BLOCK];
    int32_t pwdaq; // горизонтальные масштабы
};

#define DATA_TYPE_SIG_COMBINED_MULTI 22
#ifdef __linux__
struct __attribute__((packed)) combined_multi_offsets_t
#else
    struct combined_multi_offsets_t
#endif // __linux__
{
    int32_t data;    // Отступ начала первой порции сжатых данных, -1 если не используется
    int32_t counter; // Отступ начала массива счётчиков полезного размера (в количестве int), -1 если не используется
    int32_t re; // float re[n_smpl], суммарный канал, -1 если не используется
    int32_t im; // float im[n_smpl], суммарный канал, -1 если не используется
    int32_t pw; // uint16_t pw[n_smpl], горизонтальные масштабы, -1 если не используется
};

enum DATA_FLAG : uint32_t {
    DATA_FLAG_SIGNAL = 0x00000001,
    DATA_FLAG_REIM = 0x00000002,
    DATA_FLAG_PW = 0x00000004,
    DATA_FLAG_HASH = 0x00000008,
    DATA_FLAG_DAQ = 0x00000010,
    DATA_FLAG_PWDAQ = 0x00000020,
};

#ifdef __linux__
struct __attribute__((packed)) shm_info_t
#else
struct shm_info_t
#endif // __linux__
{
    uint32_t data_type; // DATA_TYPE_SIG_COMBINED_SIMPLE или DATA_TYPE_SIG_COMBINED_SCALED
    uint32_t data_flags;
    char hp_prefix[256]; // путь к файлам, "/mnt/ww"
    uint32_t num_devs; // количество устройств ШЦРПУ, 0..3 => pup0_<> .. pup3_<>
    uint32_t n_pup_per_dev; // количество pup файлов на устройство, pupX_000 .. pupX_<hp_per_dev-1>
    uint32_t n_sup; // количество sup файлов, 0 - если не используется (для одного устройства), sup_0 .. sup_<n_sup-1>
    uint32_t n_ch_per_wup; // каналов на устройство
    uint32_t n_frames; // количество блоков/фреймов в буфере
    int32_t n_smpl; // количество отсчетов в блоке/фрейме
    double bin_hz; // цена бина
    int32_t item_width; // порция данных для сжатия, задается в бинах
    int32_t item_stride; // 0 - сжатые ячейки лежат подряд, иначе страйд между сжатыми данными
    int32_t compress_algorithm;
    int32_t n_lines_per_block; // линий на блок, пока не исп., 1
    int32_t mips; // количество масштабов, пока не исп., -1
    uint32_t n_sum; // количество sum файлов на устройство, sumX_000 .. sumX_<hp_per_dev-1>
    uint32_t n_sig; // количество sig файлов на устройство, sigX_000 .. sigX_<hp_per_dev-1>
};

#ifdef __linux__
struct __attribute__((packed)) root_info_t
#else
struct root_info_t
#endif // __linux__
{
    uint32_t marker;
    int32_t bacos_total;
    int32_t bacos_curr;
    int32_t hp_total;  // всего hugepage`й
    int32_t hp_curr;   // номер hugepage
    int32_t ach_total; // всего каналов
    int32_t ach_start;
    int32_t ach_count;
    int32_t block_count; // количество блоков на hugepage
    int32_t n_smpl;      // количество отсчетов в блоке (фрейме)
    int32_t n_lines_per_block;
    int32_t mips; // количество масштабов для DATA_TYPE_PANO_ReImDAQPw
    uint32_t data_type;
    int32_t item_width; // порция данных для сжатия, задается в бинах
    int32_t item_stride; // 0 - сжатые ячейки лежат подряд, иначе страйд между сжатыми данными
    int32_t compress_algorithm;
    int32_t block_info_offset[MAX_BLOCKS_PER_HP]; // смещения описаний блоков (фреймов)
    union {
        sig_offsets_t sig_offsets[MAX_BLOCKS_PER_HP];
        combined_simple_offsets_t combined_simple_offsets[MAX_BLOCKS_PER_HP];
        combined_scaled_offsets_t combined_scaled_offsets[MAX_BLOCKS_PER_HP];
        combined_multi_offsets_t combined_multi_offsets[MAX_BLOCKS_PER_HP];
    };
};

enum SIG_TYPE : uint32_t {
    SIG_TYPE_INVALID = 0x00000001,
    SIG_TYPE_CALIB_N = 0x00000002,
    SIG_TYPE_CALIB_Q = 0x00000004,
    SIG_TYPE_ANT = 0x00000008,
    SIG_TYPE_ANT_SCAN = 0x00000010,
};

// общие данные для сигналов и панорамы
#ifdef __linux__
struct __attribute__((packed)) block_info_t
#else
struct block_info_t
#endif // __linux__
{
	uint32_t marker;      ///< Маркер
	uint32_t nframe;      ///< Индекс
	int32_t sec;          ///< Время записи (с)
	int32_t nsec;         ///< Время записи (нс)
	float att_dB;         ///< Значение аттеньюатора
	double l_freq_Hz;     ///< частота левой границы, Гц
	double bin_Hz;        ///< цена бина, Гц
	int32_t n_smpl;       ///< количество бинов
	double latitude;      ///< широта, рад
	double longitude;     ///< долгота, рад
	double altitude;      ///< высота, м
	float north_velocity; ///< северная составляющая скорости, м/c
	float east_velocity;  ///< восточная составляющая скорости, м/c
	float down_velocity;  ///< проекция скорости на отвес, м/c
	float heading;        ///< курс относительно севера по часовой стрелке, -pi .. +pi
	float pitch;          ///< тангаж, положительные углы от горизонта вверх, -pi/2 .. +pi/2
	float roll;           ///< крен, правое крыло вниз - крен положительный,-pi .. +pi
	float heading_corr;   ///< курс относительно севера по часовой стрелке, коррекция
	float pitch_corr;     ///< тангаж, положительные углы от горизонта вверх, коррекция
	float roll_corr;      ///< крен, правое крыло вниз - крен положительный, коррекция
	uint32_t is_afs_corrected; ///< Признак "скорректирован ли AFS"			// не копируется в WriteParams, но там присутствует
	uint32_t is_daq_corrected; ///< Признак "скорректирован ли DAQ"			// не копируется в WriteParams, но там присутствует
	int32_t ach_total;               ///< количество антенных элементов
	float afs_xyz[MAX_AFS_CHANS][3]; ///< декартовы координаты антенного элемента	// не копируется в WriteParams, но там присутствует
	uint32_t status;	  ///< Статус
	int32_t hw_nframe;     // номер фрейма от ШЦРПУ							// не копируется в WriteParams, и там отсутствует
	int32_t block_idx;     // индекс блока (0..hp_total*block_count)		// не копируется в WriteParams, и там отсутствует
	int32_t block_idx_dst; // индекс блока на машине DST (если нет, то -1)	// не копируется в WriteParams, и там отсутствует
    uint32_t rpu_MHz;      // частота РПУ, шаг 10 МГц
    uint32_t sig_type;    // enum SIG_TYPE: нет данных, калибровки или эфир
    uint32_t litera;      // номер литеры: 0 - первая (основная), 1 - вторая и т.д.
    uint32_t scan_i;      // номер интервала сканирования
    uint32_t scan_N;      // кол-во интервалов сканирования
};

#ifdef _WIN32
#pragma pack(pop)
#endif

#endif // HP_STRUCTS_H
