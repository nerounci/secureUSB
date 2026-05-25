//#include <iostream>
//#include <fstream>
//#include <vector>
//#include <string>
//#include <cstring>
//#include <ctime>
//#include <cstdlib>
//
//#include <openssl/evp.h>
//#include <openssl/rand.h>
//#include <openssl/sha.h>
//
//using namespace std; //в целом - плохо, но тут некритично
//
//// ===================== CONSTANTS =====================
//
//const int AES_KEY_SIZE = 32;
//const int AES_IV_SIZE = 16;
//const int SALT_SIZE = 16;
//const int PBKDF2_ITERS = 100000;
//
//// ===================== STEGO FOOTER =====================
//
//struct StegoFooter {
//    char marker[16];
//    uint64_t offset;
//};
//
//// ===================== UTILS =====================
//
//bool read_file(const string& path, vector<uint8_t>& out) {
//    ifstream f(path, ios::binary | ios::ate);
//    if (!f.is_open()) return false;
//
//    size_t size = f.tellg();
//    f.seekg(0);
//
//    out.resize(size);
//    f.read((char*)out.data(), size);
//    return true;
//}
//
//bool write_file(const string& path, const vector<uint8_t>& data) {
//    ofstream f(path, ios::binary);
//    if (!f.is_open()) return false;
//
//    f.write((char*)data.data(), data.size());
//    return true;
//}
//
//// ===================== TAR =====================
//
//bool create_tar(const string& folder, const string& tar_path) {
//    string cmd = "tar -cf " + tar_path + " -C " + folder + " .";
//    return system(cmd.c_str()) == 0;
//}
//
//bool extract_tar(const string& tar_path, const string& out_dir) {
//    string cmd = "mkdir -p " + out_dir + " && tar -xf " + tar_path + " -C " + out_dir;
//    return system(cmd.c_str()) == 0;
//}
//
//// ===================== CRYPTO =====================
//
//bool derive_key(const string& pass, uint8_t* salt, uint8_t* key) {
//    return PKCS5_PBKDF2_HMAC(
//        pass.c_str(),
//        pass.size(),
//        salt,
//        SALT_SIZE,
//        PBKDF2_ITERS,
//        EVP_sha256(),
//        AES_KEY_SIZE,
//        key
//    );
//}
//
//bool encrypt_data(const vector<uint8_t>& in,
//                   const uint8_t* key,
//                   const uint8_t* iv,
//                   vector<uint8_t>& out) {
//
//    EVP_CIPHER_CTX* ctx = EVP_CIPHER_CTX_new();
//
//    out.resize(in.size() + 32);
//
//    int len = 0;
//    int total = 0;
//
//    EVP_EncryptInit_ex(ctx, EVP_aes_256_cbc(), NULL, key, iv);
//
//    EVP_EncryptUpdate(ctx,
//                      out.data(),
//                      &len,
//                      in.data(),
//                      in.size());
//
//    total = len;
//
//    EVP_EncryptFinal_ex(ctx,
//                        out.data() + len,
//                        &len);
//
//    total += len;
//
//    EVP_CIPHER_CTX_free(ctx);
//
//    out.resize(total);
//    return true;
//}
//
//bool decrypt_data(const vector<uint8_t>& in,
//                   const uint8_t* key,
//                   const uint8_t* iv,
//                   vector<uint8_t>& out) {
//
//    EVP_CIPHER_CTX* ctx = EVP_CIPHER_CTX_new();
//
//    out.resize(in.size());
//
//    int len = 0;
//    int total = 0;
//
//    EVP_DecryptInit_ex(ctx, EVP_aes_256_cbc(), NULL, key, iv);
//
//    if (!EVP_DecryptUpdate(ctx,
//                           out.data(),
//                           &len,
//                           in.data(),
//                           in.size())) {
//        return false;
//    }
//
//    total = len;
//
//    if (!EVP_DecryptFinal_ex(ctx,
//                            out.data() + len,
//                            &len)) {
//        return false;
//    }
//
//    total += len;
//
//    EVP_CIPHER_CTX_free(ctx);
//
//    out.resize(total);
//    return true;
//}
//
//// ===================== CREATE =====================
//
//bool create_container(const string& folder,
//                      const string& cover,
//                      const string& output,
//                      const string& password) {
//
//    cout << "[*] Creating TAR...\n";
//
//    string tar = "/tmp/container.tar";
//
//    if (!create_tar(folder, tar)) {
//        cout << "\e[0;101mtar failed\e[0m\n";
//        return false;
//    }
//
//    vector<uint8_t> tar_data;
//    if (!read_file(tar, tar_data)) return false;
//
//    uint8_t salt[SALT_SIZE];
//    uint8_t iv[AES_IV_SIZE];
//    uint8_t key[AES_KEY_SIZE];
//
//    RAND_bytes(salt, SALT_SIZE);
//    RAND_bytes(iv, AES_IV_SIZE);
//
//    derive_key(password, salt, key);
//
//    vector<uint8_t> encrypted;
//    encrypt_data(tar_data, key, iv, encrypted);
//
//    vector<uint8_t> cover_data;
//    if (!read_file(cover, cover_data)) return false;
//
//    ofstream out(output, ios::binary);
//
//    out.write((char*)cover_data.data(), cover_data.size());
//    out.write((char*)iv, AES_IV_SIZE);
//    out.write((char*)salt, SALT_SIZE);
//    out.write((char*)encrypted.data(), encrypted.size());
//
//    StegoFooter footer;
//    memcpy(footer.marker, "STEGO_SEC_2026", 15);
//    footer.offset = cover_data.size();
//
//    out.write((char*)&footer, sizeof(footer));
//
//    cout << "\e[0;102m[+] Container created\e[0m\n";
//    return true;
//}
//
//// ===================== OPEN =====================
//
//bool open_container(const string& file,
//                    const string& password) {
//
//    vector<uint8_t> data;
//    if (!read_file(file, data)) return false;
//
//    StegoFooter footer;
//    memcpy(&footer,
//           data.data() + data.size() - sizeof(footer),
//           sizeof(footer));
//
//    uint64_t offset = footer.offset;
//
//    uint8_t* iv = data.data() + offset;
//    uint8_t* salt = iv + AES_IV_SIZE;
//    uint8_t* enc = salt + SALT_SIZE;
//
//    size_t enc_size =
//        data.size() - offset - AES_IV_SIZE - SALT_SIZE - sizeof(footer);
//
//    uint8_t key[AES_KEY_SIZE];
//    derive_key(password, salt, key);
//
//    vector<uint8_t> decrypted;
//
//    if (!decrypt_data(
//            vector<uint8_t>(enc, enc + enc_size),
//            key,
//            iv,
//            decrypted)) {
//
//        cout << "\e[0;101mdecrypt failed (wrong password?)\e[0m\n";
//        return false;
//    }
//
//    string tar = "/tmp/out.tar";
//    write_file(tar, decrypted);
//
//    extract_tar(tar, "./extracted");
//
//    cout << "[+] Extracted successfully\n";
//    return true;
//}
//
//// ===================== MAIN =====================
//
//int main() {
//
//    cout << "Secure Storage System\n";
//
//    int c;
//    string folder, cover, output, file, pass;
//
//    while (true) {
//
//        cout << "\n1 Create\n2 Open\n0 Exit\n> ";
//        cin >> c;
//
//        if (c == 0) break;
//
//        if (c == 1) {
//            cout << "Folder: ";
//            cin >> folder;
//            cout << "Cover file: ";
//            cin >> cover;
//            cout << "Output: ";
//            cin >> output;
//            cout << "Password: ";
//            cin >> pass;
//
//            create_container(folder, cover, output, pass);
//        }
//
//        if (c == 2) {
//            cout << "File: ";
//            cin >> file;
//            cout << "Password: ";
//            cin >> pass;
//
//            open_container(file, pass);
//        }
//    }
//
//    return 0;
//}
//


#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <cstring>
#include <ctime>
#include <cstdlib>
#include <iomanip>
#include <sstream>

#include <openssl/evp.h>
#include <openssl/rand.h>
#include <openssl/sha.h>

using namespace std; //в целом - плохо, но тут некритично



const int AES_KEY_SIZE = 32;
const int AES_IV_SIZE = 16;
const int SALT_SIZE = 16;
const int PBKDF2_ITERS = 100000;

const string LOG_FILENAME = ".container.log";
const string FAILED_AUTH_LOG = ".failed_auth.log"; // Внешний лог для неудачных попыток



struct StegoFooter {
    char marker[16];
    uint64_t offset;
};



struct LogEntry {
    time_t timestamp;
    char event_type[32];  // "OPEN", "CLOSE", "CREATE", "ADD_FILE", "DELETE_FILE", "AUTH_FAIL"
    char details[128];
};



string get_timestamp() {
    time_t now = time(nullptr);
    struct tm* tm_info = localtime(&now);
    char buffer[64];
    strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", tm_info);
    return string(buffer);
}

bool read_file(const string& path, vector<uint8_t>& out) {
    ifstream f(path, ios::binary | ios::ate);
    if (!f.is_open()) return false;

    size_t size = f.tellg();
    f.seekg(0);

    out.resize(size);
    f.read((char*)out.data(), size);
    return true;
}

bool write_file(const string& path, const vector<uint8_t>& data) {
    ofstream f(path, ios::binary);
    if (!f.is_open()) return false;

    f.write((char*)data.data(), data.size());
    return true;
}



// Логирование внутри контейнера (зашифрованное)
bool append_log_internal(const string& temp_dir, const string& event_type, const string& details) {
    string log_path = temp_dir + "/" + LOG_FILENAME;
    
    ofstream log_file(log_path, ios::app);
    if (!log_file.is_open()) return false;
    
    log_file << "[" << get_timestamp() << "] "
             << event_type << ": "
             << details << "\n";
    
    log_file.close();
    return true;
}


bool log_failed_auth(const string& container_path) {
    ofstream log_file(FAILED_AUTH_LOG, ios::app);
    if (!log_file.is_open()) return false;
    
    log_file << "[" << get_timestamp() << "] "
             << "FAILED_AUTH_ATTEMPT on container: "
             << container_path << "\n";
    
    log_file.close();
    return true;
}

// Просмотр логов контейнера
void display_log(const string& temp_dir) {
    string log_path = temp_dir + "/" + LOG_FILENAME;
    
    ifstream log_file(log_path);
    if (!log_file.is_open()) {
        cout << "[i] No log entries found\n";
        return;
    }
    
    cout << "\n\e[1;93m===== CONTAINER LOG =====\e[0m\n";
    string line;
    while (getline(log_file, line)) {
        cout << line << "\n";
    }
    cout << "\e[1;93m========================\e[0m\n\n";
}



bool create_tar(const string& folder, const string& tar_path) {
    string cmd = "tar -cf " + tar_path + " -C " + folder + " .";
    return system(cmd.c_str()) == 0;
}

bool extract_tar(const string& tar_path, const string& out_dir) {
    string cmd = "mkdir -p " + out_dir + " && tar -xf " + tar_path + " -C " + out_dir;
    return system(cmd.c_str()) == 0;
}



bool derive_key(const string& pass, uint8_t* salt, uint8_t* key) {
    return PKCS5_PBKDF2_HMAC(
        pass.c_str(),
        pass.size(),
        salt,
        SALT_SIZE,
        PBKDF2_ITERS,
        EVP_sha256(),
        AES_KEY_SIZE,
        key
    );
}

bool encrypt_data(const vector<uint8_t>& in,
                   const uint8_t* key,
                   const uint8_t* iv,
                   vector<uint8_t>& out) {

    EVP_CIPHER_CTX* ctx = EVP_CIPHER_CTX_new();

    out.resize(in.size() + 32);

    int len = 0;
    int total = 0;

    EVP_EncryptInit_ex(ctx, EVP_aes_256_cbc(), NULL, key, iv);

    EVP_EncryptUpdate(ctx,
                      out.data(),
                      &len,
                      in.data(),
                      in.size());

    total = len;

    EVP_EncryptFinal_ex(ctx,
                        out.data() + len,
                        &len);

    total += len;

    EVP_CIPHER_CTX_free(ctx);

    out.resize(total);
    return true;
}

bool decrypt_data(const vector<uint8_t>& in,
                   const uint8_t* key,
                   const uint8_t* iv,
                   vector<uint8_t>& out) {

    EVP_CIPHER_CTX* ctx = EVP_CIPHER_CTX_new();

    out.resize(in.size());

    int len = 0;
    int total = 0;

    EVP_DecryptInit_ex(ctx, EVP_aes_256_cbc(), NULL, key, iv);

    if (!EVP_DecryptUpdate(ctx,
                           out.data(),
                           &len,
                           in.data(),
                           in.size())) {
        EVP_CIPHER_CTX_free(ctx);
        return false;
    }

    total = len;

    if (!EVP_DecryptFinal_ex(ctx,
                            out.data() + len,
                            &len)) {
        EVP_CIPHER_CTX_free(ctx);
        return false;
    }

    total += len;

    EVP_CIPHER_CTX_free(ctx);

    out.resize(total);
    return true;
}



bool create_container(const string& folder,
                      const string& cover,
                      const string& output,
                      const string& password) {

    cout << "[*] Creating TAR...\n";

    // Создаем временную директорию для подготовки контейнера
    string temp_dir = "/tmp/container_temp_" + to_string(time(nullptr));
    system(("mkdir -p " + temp_dir).c_str());
    system(("cp -r " + folder + "/* " + temp_dir + "/").c_str());
    
    // Добавляем первую запись в лог
    append_log_internal(temp_dir, "CREATE", "Container created");

    string tar = "/tmp/container.tar";

    if (!create_tar(temp_dir, tar)) {
        cout << "\e[0;101mtar failed\e[0m\n";
        system(("rm -rf " + temp_dir).c_str());
        return false;
    }

    system(("rm -rf " + temp_dir).c_str());

    vector<uint8_t> tar_data;
    if (!read_file(tar, tar_data)) return false;

    uint8_t salt[SALT_SIZE];
    uint8_t iv[AES_IV_SIZE];
    uint8_t key[AES_KEY_SIZE];

    RAND_bytes(salt, SALT_SIZE);
    RAND_bytes(iv, AES_IV_SIZE);

    derive_key(password, salt, key);

    vector<uint8_t> encrypted;
    encrypt_data(tar_data, key, iv, encrypted);

    vector<uint8_t> cover_data;
    if (!read_file(cover, cover_data)) return false;

    ofstream out(output, ios::binary);

    out.write((char*)cover_data.data(), cover_data.size());
    out.write((char*)iv, AES_IV_SIZE);
    out.write((char*)salt, SALT_SIZE);
    out.write((char*)encrypted.data(), encrypted.size());

    StegoFooter footer;
    memcpy(footer.marker, "STEGO_SEC_2026", 15);
    footer.offset = cover_data.size();

    out.write((char*)&footer, sizeof(footer));

    cout << "\e[0;102m[+] Container created\e[0m\n";
    return true;
}



bool open_container(const string& file,
                    const string& password,
                    string& temp_dir_out) {

    vector<uint8_t> data;
    if (!read_file(file, data)) return false;

    StegoFooter footer;
    memcpy(&footer,
           data.data() + data.size() - sizeof(footer),
           sizeof(footer));

    uint64_t offset = footer.offset;

    uint8_t* iv = data.data() + offset;
    uint8_t* salt = iv + AES_IV_SIZE;
    uint8_t* enc = salt + SALT_SIZE;

    size_t enc_size =
        data.size() - offset - AES_IV_SIZE - SALT_SIZE - sizeof(footer);

    uint8_t key[AES_KEY_SIZE];
    derive_key(password, salt, key);

    vector<uint8_t> decrypted;

    if (!decrypt_data(
            vector<uint8_t>(enc, enc + enc_size),
            key,
            iv,
            decrypted)) {

        cout << "\e[0;101mdecrypt failed (wrong password?)\e[0m\n";
        log_failed_auth(file);
        return false;
    }

    string tar = "/tmp/out.tar";
    write_file(tar, decrypted);

    temp_dir_out = "/tmp/extracted_" + to_string(time(nullptr));
    
    if (!extract_tar(tar, temp_dir_out)) {
        return false;
    }

    // Логируем успешное открытие
    append_log_internal(temp_dir_out, "OPEN", "Container opened successfully");

    cout << "\e[0;102m[+] Container opened\e[0m\n";
    cout << "[i] Extracted to: " << temp_dir_out << "\n";
    
    return true;
}



bool repack_container(const string& temp_dir,
                      const string& original_file,
                      const string& password) {
    
    cout << "[*] Repacking container...\n";

    // Логируем закрытие
    append_log_internal(temp_dir, "CLOSE", "Container closed and repacked");

    // Читаем оригинальный файл для извлечения cover
    vector<uint8_t> orig_data;
    if (!read_file(original_file, orig_data)) return false;

    StegoFooter footer;
    memcpy(&footer,
           orig_data.data() + orig_data.size() - sizeof(footer),
           sizeof(footer));

    uint64_t cover_size = footer.offset;
    vector<uint8_t> cover_data(orig_data.begin(), orig_data.begin() + cover_size);

    // Создаем новый TAR из измененной директории
    string tar = "/tmp/repack.tar";
    if (!create_tar(temp_dir, tar)) {
        cout << "\e[0;101mtar failed\e[0m\n";
        return false;
    }

    vector<uint8_t> tar_data;
    if (!read_file(tar, tar_data)) return false;

    // Извлекаем оригинальные соль и IV из старого контейнера
    uint8_t* orig_iv = orig_data.data() + cover_size;
    uint8_t* orig_salt = orig_iv + AES_IV_SIZE;

    uint8_t key[AES_KEY_SIZE];
    derive_key(password, orig_salt, key);

    // Шифруем новый TAR с теми же параметрами
    vector<uint8_t> encrypted;
    encrypt_data(tar_data, key, orig_iv, encrypted);

    // Записываем обновленный контейнер
    ofstream out(original_file, ios::binary);
    out.write((char*)cover_data.data(), cover_data.size());
    out.write((char*)orig_iv, AES_IV_SIZE);
    out.write((char*)orig_salt, SALT_SIZE);
    out.write((char*)encrypted.data(), encrypted.size());

    StegoFooter new_footer;
    memcpy(new_footer.marker, "STEGO_SEC_2026", 15);
    new_footer.offset = cover_size;

    out.write((char*)&new_footer, sizeof(new_footer));

    cout << "\e[0;102m[+] Container repacked successfully\e[0m\n";
    return true;
}



bool add_file_to_container(const string& temp_dir, const string& file_path) {
    string filename = file_path.substr(file_path.find_last_of("/\\") + 1);
    string dest = temp_dir + "/" + filename;
    
    string cmd = "cp " + file_path + " " + dest;
    if (system(cmd.c_str()) != 0) {
        cout << "\e[0;101mFailed to copy file\e[0m\n";
        return false;
    }

    append_log_internal(temp_dir, "ADD_FILE", "Added: " + filename);
    cout << "\e[0;102m[+] File added: " << filename << "\e[0m\n";
    return true;
}

bool delete_file_from_container(const string& temp_dir, const string& filename) {
    string file_path = temp_dir + "/" + filename;
    
    if (remove(file_path.c_str()) != 0) {
        cout << "\e[0;101mFailed to delete file\e[0m\n";
        return false;
    }

    append_log_internal(temp_dir, "DELETE_FILE", "Deleted: " + filename);
    cout << "\e[0;102m[+] File deleted: " << filename << "\e[0m\n";
    return true;
}

void list_files_in_container(const string& temp_dir) {
    cout << "\n\e[1;96m===== FILES IN CONTAINER =====\e[0m\n";
    string cmd = "ls -lh " + temp_dir + " | grep -v '^total' | grep -v '^d'";
    system(cmd.c_str());
    cout << "\e[1;96m==============================\e[0m\n\n";
}



int main() {

    cout << "\e[1;96m╔═══════════════════════════════════╗\e[0m\n";
    cout << "\e[1;96m║       Secure Storage System       ║\e[0m\n";
    cout << "\e[1;96m╚═══════════════════════════════════╝\e[0m\n";

    int c;
    string folder, cover, output, file, pass;
    string current_container, current_temp_dir, current_password;
    bool container_open = false;

    while (true) {
        cout << "1. Create Container\n";
        cout << "2. Open Container\n";
        cout << "3. List Files (requires open container)\n";
        cout << "4. Add File (requires open container)\n";
        cout << "5. Delete File (requires open container)\n";
        cout << "6. View Log (requires open container)\n";
        cout << "7. Close & Save Container\n";
        cout << "0. Exit\n";
        
        if (container_open) {
            cout << "\e[0;102m[Container: " << current_container << " - OPEN]\e[0m\n";
        }
        
        cout << "> ";
        cin >> c;

        if (c == 0) {
            if (container_open) {
                cout << "\e[0;103m[!] Warning: Container is still open. Close it first? (y/n): \e[0m";
                char choice;
                cin >> choice;
                if (choice == 'y' || choice == 'Y') {
                    repack_container(current_temp_dir, current_container, current_password);
                    system(("rm -rf " + current_temp_dir).c_str());
                }
            }
            break;
        }

        if (c == 1) {
            cout << "Folder: ";
            cin >> folder;
            cout << "Cover file: ";
            cin >> cover;
            cout << "Output: ";
            cin >> output;
            cout << "Password: ";
            cin >> pass;

            create_container(folder, cover, output, pass);
        }

        else if (c == 2) {
            if (container_open) {
                cout << "\e[0;103m[!] Close current container first\e[0m\n";
                continue;
            }

            cout << "File: ";
            cin >> file;
            cout << "Password: ";
            cin >> pass;

            string temp_dir;
            if (open_container(file, pass, temp_dir)) {
                current_container = file;
                current_temp_dir = temp_dir;
                current_password = pass;
                container_open = true;
            }
        }

        else if (c == 3) {
            if (!container_open) {
                cout << "\e[0;101m[!] No container open\e[0m\n";
                continue;
            }
            list_files_in_container(current_temp_dir);
        }

        else if (c == 4) {
            if (!container_open) {
                cout << "\e[0;101m[!] No container open\e[0m\n";
                continue;
            }
            
            cout << "File path to add: ";
            string file_to_add;
            cin >> file_to_add;
            
            add_file_to_container(current_temp_dir, file_to_add);
        }

        else if (c == 5) {
            if (!container_open) {
                cout << "\e[0;101m[!] No container open\e[0m\n";
                continue;
            }
            
            cout << "Filename to delete: ";
            string file_to_delete;
            cin >> file_to_delete;
            
            delete_file_from_container(current_temp_dir, file_to_delete);
        }

        else if (c == 6) {
            if (!container_open) {
                cout << "\e[0;101m[!] No container open\e[0m\n";
                continue;
            }
            
            display_log(current_temp_dir);
        }

        else if (c == 7) {
            if (!container_open) {
                cout << "\e[0;101m[!] No container open\e[0m\n";
                continue;
            }
            
            repack_container(current_temp_dir, current_container, current_password);
            system(("rm -rf " + current_temp_dir).c_str());
            
            container_open = false;
            current_container = "";
            current_temp_dir = "";
            current_password = "";
        }
    }

    return 0;
}
