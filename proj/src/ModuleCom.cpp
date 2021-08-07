#include "ModuleCom.h"

#include <zephyr.h>
#include <device.h>
#include <devicetree.h>
#include <sys/printk.h>
#include <shell/shell.h>
#include <drivers/uart.h>

#define UART_NODE DT_LABEL(DT_ALIAS(intermdule))

LOG_MODULE_REGISTER(ModCom, LOG_LEVEL_INF);

struct FrameData {
	struct Buffer {
		uint8_t buffer[sizeof(FuncFrame)];
		size_t idx;
	} in, frame;
	struct BufferOut {
		union {
			uint8_t buffer[sizeof(FuncFrame)];
			FuncFrame frame;
		};
		size_t idx;
		size_t length;
	} out;
	int64_t lastDataTime;
	int64_t frameDuration;
};



typedef void (*ModuleComCallI32)(const int32_t value);
volatile uint32_t configValues[ConfigValue::SIZE] = {0,};

void ModuleComCall_0_readback(const int32_t idx) {
	LOG_INF("[dataParser] ModuleComCall_0_readback");

	FuncFrame frame;
	frame.func = 0;
	if(idx < ConfigValue::SIZE)
	{
		frame.subFunc = idx;
		frame.i32 = configValues[idx];
		dataSendFrame(frame);
	}else{
		frame.subFunc = 255U;
		frame.i32 = configValues[idx];
		dataSendFrame(frame);
	}
}

void ModuleComCall_1(const int32_t value) {
	LOG_INF("[dataParser] ModuleComCall_1");
	configValues[1] = value;
}

void ModuleComCall_2(const int32_t value) {
	LOG_INF("[dataParser] ModuleComCall_2");
	configValues[2] = value;
}

const ModuleComCallI32 calls[] = {
	ModuleComCall_0_readback,
	ModuleComCall_1,
	ModuleComCall_2
};




K_SEM_DEFINE(semDataOut, 1, 1);
K_SEM_DEFINE(semDataIn, 0, 1);
static void uart_fifo_callback(const struct device *dev, void *user_data)
{
	struct FrameData* buffer = (struct FrameData*)user_data;

	if (!uart_irq_update(dev)) {
		LOG_WRN("UART IRQ Fail");
		return;
	}

	if (uart_irq_tx_ready(dev)) {
    if(buffer->out.idx >= buffer->out.length) {
			k_sem_give(&semDataOut);
		  uart_irq_tx_disable(dev);
		} else {
			uart_fifo_fill(dev, &buffer->out.buffer[buffer->out.idx++], 1);
		}
	}

	if (uart_irq_rx_ready(dev)) {
		uint8_t recvData;
		uart_fifo_read(dev, &recvData, 1);
		int64_t now = k_uptime_get();
		if((buffer->lastDataTime + buffer->frameDuration) < now){
			buffer->in.idx = 0;
			for(size_t idx = 0; idx < sizeof(buffer->in.buffer)-1; idx++) {
				buffer->in.buffer[idx] = 0;
			}
		}

		if(buffer->in.idx == 0) 
			buffer->lastDataTime = now;

		if(buffer->in.idx < sizeof(buffer->in.buffer)-1) {
			buffer->in.buffer[buffer->in.idx++] = recvData;
		} else {
			for(size_t idx = 0; idx < sizeof(buffer->in.buffer); idx++) {
				if(idx < buffer->in.idx)
					buffer->frame.buffer[idx] = buffer->in.buffer[idx];
				else
					buffer->frame.buffer[idx] = 0;
			}
			buffer->frame.idx = buffer->in.idx;
			buffer->in.idx = 0;
			k_sem_give(&semDataIn);
		}
	}
}






struct FrameData frameData;
const struct device *uart_dev = device_get_binding(UART_NODE);


bool dataSendFrame(const FuncFrame& frame, const float timeout)
{
	return dataSend(reinterpret_cast<const uint8_t*>(&frame), sizeof(frame), timeout);
}

bool dataSend(const uint8_t* const data, const size_t length, float timeout)
{
	if(k_sem_take(&semDataOut, K_MSEC(timeout*1000)) != 0)
		return false;

  if(length > sizeof(frameData.out.buffer)) {
    frameData.out.length = sizeof(frameData.out.buffer);
  } else {
    frameData.out.length = length;
  }
  
  for(size_t idx = 0; idx < frameData.out.length; idx++)
  {
    frameData.out.buffer[idx] = data[idx];
  }
  frameData.out.idx = 0;
	uart_irq_tx_enable(uart_dev);

	return true; 
}



void dataParser(void)
{
	LOG_INF("[dataParser] Init");
	if(!device_is_ready(uart_dev)) {
		LOG_ERR("Can't setup " UART_NODE "");
		return;
	}

	frameData.frameDuration = 10;
	uart_irq_callback_user_data_set(uart_dev, uart_fifo_callback, &frameData);
	uart_irq_rx_enable(uart_dev);

	LOG_INF("[dataParser] Run");
	for (size_t loopIter = 0;;loopIter++) {
		if (k_sem_take(&semDataIn, K_MSEC(10000)) != 0) {
			LOG_INF("[dataParser] Timeout");
			LOG_HEXDUMP_DBG(frameData.in.buffer, sizeof(frameData.in.buffer), "Buffer");
    } else {
			FuncFrame* funcFrame = reinterpret_cast<FuncFrame*>(frameData.in.buffer);
			printk("[dataParser] Fun: %d, SubFun: %d, Val: %08x\n",
				funcFrame->func, funcFrame->subFunc, funcFrame->i32);

			if(funcFrame->func == 0)
				if(funcFrame->subFunc < sizeof(calls)/sizeof(calls[0]))
					calls[funcFrame->subFunc](funcFrame->i32);
			LOG_HEXDUMP_DBG(frameData.frame.buffer, sizeof(frameData.frame.buffer), "Buffer");
		}
	}
}


K_THREAD_DEFINE(dataParser_id, 1024, dataParser, NULL, NULL, NULL, 7, 0, 0);
