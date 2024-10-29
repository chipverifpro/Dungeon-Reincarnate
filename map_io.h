// map_io.h

// GLOBAL Variables

// Function Prototypes
extern int read_map_csv (int load_map_number); // Read map
  extern char *load_line(FILE *file);
  extern void chomp(char *str);
  extern void split_csv_line_offsets(void);
  extern uint32_t parse_hex(char *hex_str);
  extern int print_raw_map(void);
extern int write_map_csv (void);               // Write current map

extern int read_objects_csv (char *filename);  // Read objects
  // obsolete the following version... used in read_objects_csv
  extern int split_csv_line(const char *line, char **substrings, int max_substrings);
extern int write_objects_csv (char *filename); // Write objects

struct object_s *find_uid_owner(int uid);
extern char *str_alloc_copy(char *src);
extern char *str_realloc_copy(char *src, char *old_dest);

// called by main
  void print_all_detailed_objects(void);
    void print_detailed_object (struct object_s *obj);

