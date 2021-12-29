#define EIDSP_QUANTIZE_FILTERBANK 0

#include <PDM.h>
#include <MyTugasAkhir101_inferencing.h>

typedef struct
{
    int16_t *buffer;
    uint8_t buf_ready;
    uint32_t buf_count;
    uint32_t n_samples;
} inference_t;

static inference_t inference;
static signed short sampleBuffer[2048];
static bool debug_nn = false;
unsigned int sample;
unsigned int analog;
unsigned long previousMillis = 0;
const long interval = 1000;
unsigned long ledPreviousMillis = 0;
const long ledInterval = 100;
int ledState = LOW; 
unsigned long state = 0;
String spacer = "qz";
String classification = "";

void setup()
{
    pinMode(2, OUTPUT);
    pinMode(3, OUTPUT);

    Serial.begin(115200);
    Serial1.begin(115200);
    Serial.println("Edge Impulse Inferencing Demo");

    _printf("Inferencing settings:\n");
    _printf("\tInterval: %.2f ms.\n", (float)EI_CLASSIFIER_INTERVAL_MS);
    _printf("\tFrame size: %d\n", EI_CLASSIFIER_DSP_INPUT_FRAME_SIZE);
    _printf("\tSample length: %d ms.\n", EI_CLASSIFIER_RAW_SAMPLE_COUNT / 16);
    _printf("\tNo. of classes: %d\n", sizeof(ei_classifier_inferencing_categories) / sizeof(ei_classifier_inferencing_categories[0]));

    if (microphone_inference_start(EI_CLASSIFIER_RAW_SAMPLE_COUNT) == false)
    {
        printf("ERR: Failed to setup audio sampling\r\n");
        return;
    }
}

void loop()
{
    classification = "";
    digitalWrite(2, HIGH);
    unsigned long currentMillis = millis();
    unsigned long blinkMillis = millis();
    analog = analogRead(A1);
    sample = map(analog, 0, 320, 30, 120);
    if (state > 20){
          ledState = HIGH;
    }
    if (currentMillis - previousMillis >= interval)
    {
        previousMillis = currentMillis;     
        state ++;
        if (sample >= 61)
        {
            
            bool m = microphone_inference_record();
            if (!m)
            {
                _printf("ERR: Failed to record audio...\n");
                return;
            }

            _printf("Recording done\n");

            signal_t signal;
            signal.total_length = EI_CLASSIFIER_RAW_SAMPLE_COUNT;
            signal.get_data = &microphone_audio_signal_get_data;
            ei_impulse_result_t result = {0};

            EI_IMPULSE_ERROR r = run_classifier(&signal, &result, debug_nn);
            if (r != EI_IMPULSE_OK)
            {
                _printf("ERR: Failed to run classifier (%d)\n", r);
                return;
            }

            // print the predictions
            _printf("Predictions ");
            _printf("(DSP: %d ms., Classification: %d ms., Anomaly: %d ms.)",
                   result.timing.dsp, result.timing.classification, result.timing.anomaly);
            _printf(": \n");
            _printf("dB: ");
              _printf("%d\n", sample);
              S1_printf("%d", sample);
              S1_printf("&");
            for (size_t ix = 0; ix < EI_CLASSIFIER_LABEL_COUNT; ix++)
            {
                
                _printf("    %s: %.5f\n", result.classification[ix].label, result.classification[ix].value);
//                _printf("%.5f#", result.classification[ix].label, result.classification[ix].value);
                S1_printf("%.5f#", result.classification[ix].label, result.classification[ix].value);
                
            }


#if EI_CLASSIFIER_HAS_ANOMALY == 1
            _printf("    anomaly score: %.3f\n", result.anomaly);
#endif
            if(result.classification[2].value || result.classification[3].value > 0.5){
                state = 0; 
             }
        }else {
        _printf("%d", sample);
        _printf("&\n");
        S1_printf("%d", sample);
        S1_printf("&\n");
          }
    }
   if (blinkMillis - ledPreviousMillis >= ledInterval) {
   ledPreviousMillis = blinkMillis;
    if (ledState == LOW) {
      ledState = HIGH;
    } else {
      ledState = LOW;
    }
      digitalWrite(3, ledState);
  }   
    
}

void _printf(const char *format, ...)
{
    static char print_buf[1024] = {0};

    va_list args;
    va_start(args, format);
    int r = vsnprintf(print_buf, sizeof(print_buf), format, args);
    va_end(args);

    if (r > 0)
    {
        Serial.write(print_buf);
    }
}
void S1_printf(const char *format, ...)
{
    static char print_buf[1024] = {0};

    va_list args;
    va_start(args, format);
    int r = vsnprintf(print_buf, sizeof(print_buf), format, args);
    va_end(args);

    if (r > 0)
    {
        Serial1.write(print_buf);
    }
}

static void pdm_data_ready_inference_callback(void)
{
    int bytesAvailable = PDM.available();

    // read into the sample buffer
    int bytesRead = PDM.read((char *)&sampleBuffer[0], bytesAvailable);

    if (inference.buf_ready == 0)
    {
        for (int i = 0; i < bytesRead >> 1; i++)
        {
            inference.buffer[inference.buf_count++] = sampleBuffer[i];

            if (inference.buf_count >= inference.n_samples)
            {
                inference.buf_count = 0;
                inference.buf_ready = 1;
                break;
            }
        }
    }
}

static bool microphone_inference_start(uint32_t n_samples)
{
    inference.buffer = (int16_t *)malloc(n_samples * sizeof(int16_t));

    if (inference.buffer == NULL)
    {
        return false;
    }

    inference.buf_count = 0;
    inference.n_samples = n_samples;
    inference.buf_ready = 0;

    PDM.onReceive(&pdm_data_ready_inference_callback);

    PDM.setBufferSize(4096);

    if (!PDM.begin(1, EI_CLASSIFIER_FREQUENCY))
    {
        printf("Failed to start PDM!");
        microphone_inference_end();

        return false;
    }

    PDM.setGain(127);

    return true;
}

static bool microphone_inference_record(void)
{
    inference.buf_ready = 0;
    inference.buf_count = 0;

    while (inference.buf_ready == 0)
    {
        delay(10);
    }

    return true;
}

static int microphone_audio_signal_get_data(size_t offset, size_t length, float *out_ptr)
{
    numpy::int16_to_float(&inference.buffer[offset], out_ptr, length);

    return 0;
}

static void microphone_inference_end(void)
{
    PDM.end();
    free(inference.buffer);
}

#if !defined(EI_CLASSIFIER_SENSOR) || EI_CLASSIFIER_SENSOR != EI_CLASSIFIER_SENSOR_MICROPHONE
#error "Invalid model for current sensor."
#endif
