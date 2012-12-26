#include <stdio.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#pragma pack(1)
struct xres_entry {
    char name[9];
    int32_t offset;
};

int main(int argc, char** argv) {
	char* filename;
	int fd;
	struct stat st;
	int filesize;
	void* map;
	int16_t records;

	if(argc!=2) {
		printf("usage: %s <file>\n", argv[0]);
		return 1;
	}

	filename = argv[1];
	if(stat(filename, &st)) {
		perror("stat");
		return 1;
	}

	filesize = st.st_size;
	fd = open(filename, O_RDONLY);
	if(fd == -1) {
		perror("open file");
		return 1;
	}

	map = mmap(0, filesize, PROT_READ, MAP_SHARED, fd, 0);
	if(map == MAP_FAILED) {
		close(fd);
		perror("mmap");
		return 1;
	}

	records = *(int*)map;
	printf("records: %hd\n", records);
	map+=2;
	struct xres_entry* entries = map;
	int idx;
	for(idx=0; idx<records; idx++) {
		int from, to;
		from = entries[idx].offset;
		if(idx == records -1) {
			to = filesize;
		} else {
			to = entries[idx+1].offset;
		}
		printf("%5d %10s %8d %8d\n", idx, entries[idx].name, from, to);
		FILE* f = fopen(entries[idx].name, "w");
		fwrite(map+from, 1, to-from, f);
		fclose(f);		
	}
	return 0;
}
