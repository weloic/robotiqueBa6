#ifndef PROCESS_IMAGE_H
#define PROCESS_IMAGE_H

#define IMAGE_BUFFER_SIZE		640
#define MIN_LINE_WIDTH			10
#define WIDTH_SLOPE				5
#define PXTOCM					1570.0f //experimental value

typedef uint8_t logical;
#define YES   	0
#define NO		1

enum ratio{
	SMALL = 1,
	MEDIUM = 2,
	LARGE = 3
};

#define LINE_THRESHOLD 		5
#define START_LINE_WIDTH	38

void capture_image(logical);
uint16_t get_line_position(void);
void process_image_start(void);
struct Line extract_line(uint8_t *buffer, struct Line line, uint32_t mean);
uint8_t extract_barcode(uint8_t *image);
uint32_t calculate_mean(uint8_t *buffer);
uint8_t line_width_analyse(struct Line line, uint8_t width_unit);

struct Line{
	uint16_t end_pos, begin_pos;
	uint8_t width;
	bool found;
};


#endif /* PROCESS_IMAGE_H */
