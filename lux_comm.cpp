#include <cstdio>
#include <cstring>
#include <ctime>
#include <cstdlib>
// Hằng số cho ký hiệu gói tin
const char file_log[] = "task3.log";
const unsigned char bat_dau_goi = 0x0E;
const unsigned char ket_thuc_goi = 0xFE;
// Ghi lỗi ra file log kèm thời gian
void ghi_loi_dong(const char* thong_bao, int dong = -1) {
    FILE* f = fopen(file_log, "a");
    if (f) {
        time_t bay_gio = time(NULL);
        struct tm* tg = localtime(&bay_gio);
        if (dong >= 0)
            fprintf(f, "[%04d-%02d-%02d %02d:%02d:%02d] %s at line %d\n",
                    tg->tm_year + 1900, tg->tm_mon + 1, tg->tm_mday,
                    tg->tm_hour, tg->tm_min, tg->tm_sec, thong_bao, dong);
        else
            fprintf(f, "[%04d-%02d-%02d %02d:%02d:%02d] %s\n",
                    tg->tm_year + 1900, tg->tm_mon + 1, tg->tm_mday,
                    tg->tm_hour, tg->tm_min, tg->tm_sec, thong_bao);
        fclose(f);
    }
    if (dong >= 0)
        fprintf(stderr, "%s at line %d\n", thong_bao, dong);
    else
        fprintf(stderr, "%s\n", thong_bao);
}
// Kiểm tra trùng lặp dòng
bool dong_trung_lap(const char* dong, char truoc[][256], int n, int& dong_goc) {
    for (int i = 0; i < n; ++i) {
        if (strcmp(dong, truoc[i]) == 0) {
            dong_goc = i + 1;
            return true;
        }
    }
    return false;
}
// Hàm chuyển đổi CSV sang DAT
void csv_sang_dat(const char* vao, const char* ra) {
    FILE* f_vao = fopen(vao, "r");
    FILE* f_ra = fopen(ra, "w");
    if (!f_vao) {
        ghi_loi_dong("Error 01: input file not found or not accessible");
        return;
    }
    if (!f_ra) {
        ghi_loi_dong("Error 07: cannot override output file");
        fclose(f_vao);
        return;
    }
    char dong[256];
    if (!fgets(dong, sizeof(dong), f_vao)) {
        ghi_loi_dong("Error 02: invalid input file format");
        fclose(f_vao);
        fclose(f_ra);
        return;
    }
    if (strncmp(dong, "id,", 3) != 0) {
        ghi_loi_dong("Error 02: invalid input file format");
        fclose(f_vao);
        fclose(f_ra);
        return;
    }
    int dong_so = 0;
    int so_dong = 0;
    char dong_luu[10000][256];
    while (fgets(dong, sizeof(dong), f_vao)) {
        dong_so++;
        if (++so_dong > 10000) {
            ghi_loi_dong("Error 08: input file is too large");
            break;
        }
        int dong_goc;
        if (dong_trung_lap(dong, dong_luu, dong_so - 1, dong_goc)) {
            char thong_bao[100];
            sprintf(thong_bao, "Error 06: data at line %d and %d are duplicated", dong_goc, dong_so);
            ghi_loi_dong(thong_bao);
            continue;
        }
        strcpy(dong_luu[dong_so - 1], dong);
        char truong[5][50];
        int cot = 0, i = 0, j = 0;
        for (int k = 0; dong[k] && dong[k] != '\n'; k++) {
            if (dong[k] == ',') {
                truong[cot][j] = '\0';
                cot++; j = 0;
            } else {
                truong[cot][j++] = dong[k];
            }
        }
        truong[cot][j] = '\0';
        if (cot != 4) {
            ghi_loi_dong("Error 04: invalid data", dong_so);
            continue;
        }
        int id = atoi(truong[0]);
        int vitri = atoi(truong[2]);
        float lux = atof(truong[3]);
        if (id <= 0 || vitri < 0 || lux < 0.1f) {
            ghi_loi_dong("Error 04: invalid data", dong_so);
            continue;
        }
        struct tm tg = {};
        if (sscanf(truong[1], "%d:%d:%d %d:%d:%d",
            &tg.tm_year, &tg.tm_mon, &tg.tm_mday,
            &tg.tm_hour, &tg.tm_min, &tg.tm_sec) != 6) {
            ghi_loi_dong("Error 04: invalid data", dong_so);
            continue;
        }
        tg.tm_year -= 1900;
        tg.tm_mon -= 1;
        unsigned int timestamp = mktime(&tg);
        if (timestamp > time(NULL)) {
            ghi_loi_dong("Error 05: data packet error", dong_so);
            continue;
        }
        unsigned char cond = 0;
        if (strcmp(truong[4], "dark") == 0) cond = 1;
        else if (strcmp(truong[4], "good") == 0) cond = 2;
        else if (strcmp(truong[4], "bright") == 0) cond = 3;
        else {
            ghi_loi_dong("Error 04: invalid data", dong_so);
            continue;
        }
        unsigned char goi[13];
        goi[0] = 10;
        goi[1] = id;
        goi[2] = vitri;
        goi[3] = (timestamp >> 24) & 255;
        goi[4] = (timestamp >> 16) & 255;
        goi[5] = (timestamp >> 8) & 255;
        goi[6] = timestamp & 255;
        union { float f; unsigned char b[4]; } doi;
        doi.f = lux;
        goi[7] = doi.b[3];
        goi[8] = doi.b[2];
        goi[9] = doi.b[1];
        goi[10] = doi.b[0];
        goi[11] = cond;
        unsigned char checksum = 0;
        for (int i = 0; i < 12; i++) checksum += goi[i];
        checksum = (unsigned char)(~checksum + 1);
        fprintf(f_ra, "%02X %02X ", bat_dau_goi, goi[0]);
        for (int i = 1; i <= 11; i++) fprintf(f_ra, "%02X ", goi[i]);
        fprintf(f_ra, "%02X %02X\n", checksum, ket_thuc_goi);
    }
    fclose(f_vao);
    fclose(f_ra);
    printf("Da chuyen CSV sang DAT!\n");
}
// Hàm chuyển đổi DAT sang CSV
void dat_sang_csv(const char* vao, const char* ra) {
    FILE* f_vao = fopen(vao, "r");
    FILE* f_ra = fopen(ra, "w");
    if (!f_vao) {
        ghi_loi_dong("Error 01: input file not found or not accessible");
        return;
    }
    if (!f_ra) {
        ghi_loi_dong("Error 07: cannot override output file");
        fclose(f_vao);
        return;
    }
    fprintf(f_ra, "id,time,location,value,condition\n");
    char dong[256];
    int dong_so = 0;
    while (fgets(dong, sizeof(dong), f_vao)) {
        dong_so++;
        unsigned int hex[15];
        if (sscanf(dong, "%x %x %x %x %x %x %x %x %x %x %x %x %x %x %x",
                   &hex[0], &hex[1], &hex[2], &hex[3], &hex[4],
                   &hex[5], &hex[6], &hex[7], &hex[8], &hex[9],
                   &hex[10], &hex[11], &hex[12], &hex[13], &hex[14]) != 15) {
            ghi_loi_dong("Error 05: data packet error", dong_so);
            continue;
        }
        if (hex[0] != bat_dau_goi || hex[14] != ket_thuc_goi) {
            ghi_loi_dong("Error 05: data packet error", dong_so);
            continue;
        }
        unsigned char goi[13];
        for (int i = 0; i < 13; i++) goi[i] = (unsigned char)hex[i + 1];
        // Kiểm tra checksum
        unsigned char checksum = 0;
        for (int i = 0; i < 12; i++) checksum += goi[i];
        checksum = (unsigned char)(~checksum + 1);
        if (checksum != (unsigned char)hex[13]) {
            ghi_loi_dong("Error 05: data packet error", dong_so);
            continue;
        }
        // Timestamp
        unsigned int timestamp = (goi[2] << 24) | (goi[3] << 16) | (goi[4] << 8) | goi[5];
        if (timestamp > time(NULL)) {
            ghi_loi_dong("Error 05: data packet error", dong_so);
            continue;
        }
        float lux;
        union { float f; unsigned char b[4]; } doi;
        doi.b[3] = goi[6];
        doi.b[2] = goi[7];
        doi.b[1] = goi[8];
        doi.b[0] = goi[9];
        lux = doi.f;
        struct tm* tg = localtime((time_t*)&timestamp);
        char tg_chuoi[20];
        strftime(tg_chuoi, sizeof(tg_chuoi), "%Y:%m:%d %H:%M:%S", tg);
        const char* cond = "NA";
        if (goi[10] == 1) cond = "dark";
        else if (goi[10] == 2) cond = "good";
        else if (goi[10] == 3) cond = "bright";
        fprintf(f_ra, "%d,%s,%d,%.2f,%s\n", goi[0], tg_chuoi, goi[1], lux, cond);
    }
    fclose(f_vao);
    fclose(f_ra);
    printf("Da chuyen DAT sang CSV!\n");
}

int main(int argc, char* argv[]) {
    FILE* f = fopen(file_log, "w");
    if (f) fclose(f);
    if (argc != 3) {
        ghi_loi_dong("Error 03: invalid command");
        return 0;
    }
    const char* vao = argv[1];
    const char* ra = argv[2];
    int len = strlen(vao);
    if (len >= 4 && strcmp(vao + len - 4, ".csv") == 0) {
        csv_sang_dat(vao, ra);
    } else if (len >= 4 && strcmp(vao + len - 4, ".dat") == 0) {
        dat_sang_csv(vao, ra);
    } else {
        ghi_loi_dong("Error 04: unsupported input format");
    }
    return 0;
}
