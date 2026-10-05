/* Host build of src/fieldstg/fieldstg_80087DB0.c: fieldstg_spots_create is called before its definition (an implicit
 * int declaration, which the original's GCC 2.8.1 accepts and a modern gcc rejects as a conflicting type; on LP64 it would
 * also truncate the pointer). Declared here, forced in with -include, so the unit compiles unchanged. */
struct FieldstgSpots *fieldstg_spots_create(int count);
