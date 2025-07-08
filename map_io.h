// map_io.h

// GLOBAL Variables
extern int csv_num_cells;  // count of valid CSV cells in below two variables
extern char *csv_line;     // line read from csv file, modified to replace , with \0
extern int offsets[200];   // indexes to modified csv_line for each cell

// Function Prototypes

extern int read_map_csv (int load_map_number); // Read map
  extern SDL_Color color_intensity(SDL_Color src, int intensity_pct);
  extern uint32_t **malloc_map_size_32 (int x, int y);
  extern uint8_t  **malloc_map_size_8  (int x, int y);
  extern char *load_line(FILE *file);
  extern void chomp(char *str);
  extern void split_csv_line_offsets(void);
  extern uint32_t parse_hex(char *hex_str);
  extern int print_raw_map(void);
extern int write_map_csv (void);               // Write current map
  extern uint32_t ctoi (SDL_Color c);

extern int read_objects_csv (char *filename);  // Read objects
extern int write_objects_csv (char *filename); // Write objects

extern int read_locations_csv (char *filename); // Read locations

struct object_s *find_uid_owner(int uid);
extern char *str_alloc_copy(char *src);
extern char *str_realloc_copy(char *src, char *old_dest);

// called by main
  extern void print_all_detailed_objects(void);
    extern void print_detailed_object (struct object_s *obj);

extern int map_consistancy_check (void);
