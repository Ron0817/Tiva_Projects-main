
/*--------------------------------------------------------------*/
/* headstage interface                           */

int headstage_init(void);
int headstage_init_check(void);
int Bandwidth_set(uint32_t upper, uint32_t lower);
int register4_check(uint32_t upper);
int register5_check(uint32_t upper);
int register6_check(uint32_t lower);
int Frequency_set(uint32_t frequency);
int register0_check(uint32_t frequency);
