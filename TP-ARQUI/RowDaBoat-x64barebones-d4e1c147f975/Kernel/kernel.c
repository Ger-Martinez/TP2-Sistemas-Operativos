#include <stdint.h>
#include <lib.h>   // ya que necesita la funcion cpuvendor
#include <naiveConsole.h>
#include <moduleLoader.h>
#include <idtLoader.h>     // ya que necesita a load_idt

#include <screen_driver.h>   // creo que es solo para probar cosas, despues seguro se puede sacar
#include <video_driver.h>   // para incluir la funcion init_video_driver
#include <time.h>   // seguro despues lo puedo sacar

#include "MemoryManager.h"   // BORRAR DESPUES
#include "process_manager.h"  // este hay que dejarlo
#include "buddy.h" // BORRAR DESPUES

extern uint8_t text;
extern uint8_t rodata;
extern uint8_t data;
extern uint8_t bss;
extern uint8_t endOfKernelBinary;
extern uint8_t endOfKernel;

static const uint64_t PageSize = 0x1000;

static void * const sampleCodeModuleAddress = (void*)0x400000;
static void * const sampleDataModuleAddress = (void*)0x500000;

typedef int (*EntryPoint)();

extern void _hlt(void);
extern void haltcpu(void);


void clearBSS(void * bssAddress, uint64_t bssSize) {
	memset(bssAddress, 0, bssSize);
}

void * getStackBase() {
	return (void*)(
		(uint64_t)&endOfKernel
		+ PageSize * 8				//The size of the stack itself, 32KiB
		- sizeof(uint64_t)			//Begin at the top of the stack
	);
}

void * initializeKernelBinary() {
	char buffer[10];

	ncPrint("[x64BareBones]");
	ncNewline();

	ncPrint("CPU Vendor:");
	ncPrint(cpuVendor(buffer));
	ncNewline();

	ncPrint("[Loading modules]");
	ncNewline();
	void * moduleAddresses[] = {
		sampleCodeModuleAddress,
		sampleDataModuleAddress
	};

	loadModules(&endOfKernelBinary, moduleAddresses);
	ncPrint("[Done]");
	ncNewline();
	ncNewline();

	ncPrint("[Initializing kernel's binary]");
	ncNewline();

	clearBSS(&bss, &endOfKernel - &bss);

	ncPrint("  text: 0x");
	ncPrintHex((uint64_t)&text);
	ncNewline();
	ncPrint("  rodata: 0x");
	ncPrintHex((uint64_t)&rodata);
	ncNewline();
	ncPrint("  data: 0x");
	ncPrintHex((uint64_t)&data);
	ncNewline();
	ncPrint("  bss: 0x");
	ncPrintHex((uint64_t)&bss);
	ncNewline();

	ncPrint("[Done]");
	ncNewline();
	ncNewline();
	return getStackBase();
}

int main() {
	// create the first process which is the SHELL
	uint8_t first_process = create_process((uint64_t)sampleCodeModuleAddress, 1, 0);
	if(first_process == 1) {
		drawString("SHELL WAS NOT CREATED --> ABORT");
		haltcpu();
	}

	// Arqui function provided by the proffesors. Loads the IDT and fill it with int0, int20, int21 and int80
	load_idt();

	// halt the cpu until the TT interruption arrives, which will choose the Shell as the running process
	_hlt();
	drawString("If this is shown, then something is wrong! \n");
	// ((EntryPoint)sampleCodeModuleAddress)();
	return 0;
}



