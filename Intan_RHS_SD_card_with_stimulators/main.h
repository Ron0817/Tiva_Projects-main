
/*--------------------------------------------------------------*/
/* some modules in main function                       */

int read_check(uint32_t address, uint32_t expected_value);
void SPI_Send(uint32_t data_MS, uint32_t data_LS);
int uint16_len(int arr[]) { int i; for(i=0; arr[i]!='\0';) i++; return i * 2; }
