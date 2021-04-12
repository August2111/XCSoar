#include <stdint.h>
#include <stddef.h>

#if defined(_AUG_MSC_NO)
#define MEMORY_FIELD(field, field_size) \
const size_t field ##_size = field_size; \
uint8_t mem_## field ## [field_size] = {0}; \
const uint8_t * field  = mem_ ## field;
#else
#define MEMORY_FIELD(field, field_size) \
const size_t field ## _size = field_size; \
uint8_t mem_ ## field[field_size] = {0}; \
const uint8_t * field  = mem_ ## field;
#endif

#define XCSOR_INFO_SIZE 0x1000
#define LANGUAGE_SIZE 0x1000

#ifdef _MSC_VER
// MEMORY_FIELD(de_mo, LANGUAGE_SIZE)
// const size_t de_mo_size = 177645;
#endif
// MEMORY_FIELD(de_mo,LANGUAGE_SIZE)


MEMORY_FIELD(AUTHORS_gz, XCSOR_INFO_SIZE)
MEMORY_FIELD(COPYING_gz, XCSOR_INFO_SIZE)
MEMORY_FIELD(ca_mo,LANGUAGE_SIZE)MEMORY_FIELD(cs_mo,LANGUAGE_SIZE)MEMORY_FIELD(da_mo,LANGUAGE_SIZE)MEMORY_FIELD(egm96s_dem,LANGUAGE_SIZE)MEMORY_FIELD(el_mo,LANGUAGE_SIZE)MEMORY_FIELD(es_mo,LANGUAGE_SIZE)MEMORY_FIELD(fi_mo,LANGUAGE_SIZE)MEMORY_FIELD(fr_mo,LANGUAGE_SIZE)
MEMORY_FIELD(he_mo,LANGUAGE_SIZE)
MEMORY_FIELD(hr_mo,LANGUAGE_SIZE)
MEMORY_FIELD(hu_mo,LANGUAGE_SIZE)
MEMORY_FIELD(it_mo,LANGUAGE_SIZE)
MEMORY_FIELD(ja_mo,LANGUAGE_SIZE)
MEMORY_FIELD(ko_mo,LANGUAGE_SIZE)
MEMORY_FIELD(lt_mo,LANGUAGE_SIZE)
MEMORY_FIELD(nb_mo,LANGUAGE_SIZE)
MEMORY_FIELD(nl_mo,LANGUAGE_SIZE)
MEMORY_FIELD(pl_mo,LANGUAGE_SIZE)
MEMORY_FIELD(pt_mo,LANGUAGE_SIZE)
MEMORY_FIELD(pt_BR_mo,LANGUAGE_SIZE)
MEMORY_FIELD(ro_mo,LANGUAGE_SIZE)
MEMORY_FIELD(ru_mo,LANGUAGE_SIZE)
MEMORY_FIELD(sk_mo,LANGUAGE_SIZE)
MEMORY_FIELD(sl_mo,LANGUAGE_SIZE)
MEMORY_FIELD(sr_mo,LANGUAGE_SIZE)
MEMORY_FIELD(sv_mo,LANGUAGE_SIZE)
MEMORY_FIELD(tr_mo,LANGUAGE_SIZE)
MEMORY_FIELD(uk_mo,LANGUAGE_SIZE)
MEMORY_FIELD(vi_mo,LANGUAGE_SIZE)
MEMORY_FIELD(zh_CN_mo,LANGUAGE_SIZE)
MEMORY_FIELD(zh_Hant_mo,LANGUAGE_SIZE)

#if 0
extern const uint8_t cs_mo[];
extern const uint8_t da_mo[];
extern const uint8_t de_mo[];
extern const uint8_t el_mo[];
extern const uint8_t es_mo[];
extern const uint8_t fi_mo[];
extern const uint8_t fr_mo[];
extern const uint8_t he_mo[];
extern const uint8_t hr_mo[];
extern const uint8_t hu_mo[];
extern const uint8_t it_mo[];
extern const uint8_t ja_mo[];
extern const uint8_t ko_mo[];
extern const uint8_t lt_mo[];
extern const uint8_t nb_mo[];
extern const uint8_t nl_mo[];
extern const uint8_t pl_mo[];
extern const uint8_t pt_BR_mo[];
extern const uint8_t pt_mo[];
extern const uint8_t ro_mo[];
extern const uint8_t ru_mo[];
extern const uint8_t sk_mo[];
extern const uint8_t sl_mo[];
extern const uint8_t sr_mo[];
extern const uint8_t sv_mo[];
extern const uint8_t tr_mo[];
extern const uint8_t uk_mo[];
extern const uint8_t vi_mo[];
extern const uint8_t zh_CN_mo[];
extern const uint8_t zh_Hant_mo[];
#endif
