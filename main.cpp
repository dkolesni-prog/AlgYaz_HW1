#include <iostream>

#include <string>

#include <fstream>

#include <utility>
#include <vector>

#include <ctype.h>

#include <cstdint>

#define BLOCK_SIZE 12

#define BLOCK_SHIFT 6

//Читаем с файла потоком и записываем все в 1 строку
//Делаем строковую гамму такой же длины
//Бьем строку на блоки заданного размера и делаем массив структур блока и для строки и для гаммы
//Берем 1 блок строки и 1 блок гаммы и посимвольно иксорим каждый символ строки с символом гаммы
//Каждый символ сдвигам циклически на Н бит вправа(влево) и приводим снова к чару
//Записываем все блоки в файл

struct Block {
    std::uint8_t data[BLOCK_SIZE];
};

Block operator ^ (Block data, Block gamma) {
    Block result;
    for (int i = 0; i < BLOCK_SIZE; ++i) {
       result.data[i] = data.data[i] ^ gamma.data[i];
    }
    return result;
}

Block operator << (Block data, int shift) {
    Block result;
    for (int i = 0; i < BLOCK_SIZE; ++i) {
        result.data[i] = (data.data[i] << shift) | (data.data[i] >> (8 - shift));
    }
    return result;
}


Block operator >> (Block data, int shift) {
    Block result;
    for (int i = 0; i < BLOCK_SIZE; ++i) {
        result.data[i] = (data.data[i] >> shift) | (data.data[i] << (8 - shift));
    }
    return result;
}

Block generate_gamma (int size_of_block, int key){
    int r;
    srand (time(NULL) + key);
    Block block;
    for (int i = 0; i < size_of_block; ++i) {
        r = rand();
        block.data[i] = (uint8_t) r;
    }
    return block;
}

std::vector <Block> generate_gammas (int blocks_amount){
    std::vector <Block> vector_of_gammas;
    for (int i = 0; i < blocks_amount; ++i) {
        vector_of_gammas.push_back(generate_gamma(BLOCK_SIZE, i));
    }
    return vector_of_gammas;
}

std::vector<Block> generate_data_blocks (std::string data){
    int num = data.length() / BLOCK_SIZE; // определилось количество блоков
    std::vector <Block> my_vector; // vector of blocks was created
    int j = 0; //position for substr var
    for (int i = 0; i < num; ++i) {
        Block element; // a block was created. looking nowhere
        std::string current_substr = data.substr(j + 0, j + 12);
        for (int k = 0; k < BLOCK_SIZE; ++k) {
            element.data[k] = (uint8_t)current_substr[k];
        }
        j = j + 12;
        my_vector.push_back(element); //fillin vector with blocks
    }
    return my_vector;
}

std::string stringed_block (Block result){
    std::string output;
    for (int i = 0; i < BLOCK_SIZE; ++i) {
        output += (result.data[i]);
    }
    return output ;
}

std::vector <Block> cypher (std::vector<Block> data_blocks, std::vector<Block> gamma_blocks ){
    std::vector<Block> result;
    for (int i = 0; i < data_blocks.size(); ++i) {
        Block cypher_block = (data_blocks[i] ^ gamma_blocks[i]) << BLOCK_SHIFT;
        result.push_back(cypher_block);
    }
    return result;
}

std::vector <Block> uncypher (std::vector<Block> data_blocks, std::vector<Block> gamma_blocks ){
    std::vector<Block> result;
    for (int i = 0; i < data_blocks.size(); ++i) {
        Block cypher_block = (data_blocks[i] >> BLOCK_SHIFT) ^ gamma_blocks[i];
        result.push_back(cypher_block);
    }
    return result;
}

std::vector <Block> encryption (std::vector<Block> vec_of_blocks, std::vector<Block> vec_of_gammas) {
    std::vector<Block> result = cypher( std::move(vec_of_blocks), std::move(vec_of_gammas));
    return result;
}

std::vector <Block> decryption (std::vector<Block> vec_of_blocks, std::vector<Block> vec_of_gammas) {
    std::vector<Block> result = uncypher( std::move(vec_of_blocks), std::move(vec_of_gammas));
    return result;
}


std::ofstream& operator << (std::ofstream& out, std::vector<Block>& data_blocks) {
    for (auto uncyphred_block : data_blocks) {
        out << stringed_block(uncyphred_block);
    }
    return out;
}

std::ifstream& operator >> (std::ifstream& from, std::vector<Block>& data_blocks) {
    std::string text, tmp;
    if (from.is_open()){
        while (getline(from, tmp)) {
            std::cout << tmp << std::endl;
            text += tmp; //Читаем с файла потоком и записываем все в 1 строку
        }
    }
    data_blocks = generate_data_blocks(text);
    return from;
}

int main() {
    std::vector<Block> data_blocks;
    //открытие потокв чтения и запись из файла в переменную типа вектор блоков
    std::ifstream in;
    in.open ("/Users/daniilkolesnik/CLionProjects/AlgYaz/HW#1/in.txt");
    in >> data_blocks;
    in.close();

    std::vector<Block> vec_of_gammas = generate_gammas(data_blocks.size());
    std::vector <Block> cyphred_data = encryption(data_blocks, vec_of_gammas);

    std::ofstream out;
    out.open ("/Users/daniilkolesnik/CLionProjects/AlgYaz/HW#1/cyphred.txt");
    out << cyphred_data; //запись cyphred_data в файл
    out.close();

    std::vector<Block> cyphred_data_blocks;
    in.open ("/Users/daniilkolesnik/CLionProjects/AlgYaz/HW#1/cyphred.txt");
    in >> cyphred_data_blocks; //открыли зашифрованное и записали в переменную cyphred_data_blocks
    in.close();

    std::vector <Block> uncyphred_data = decryption(cyphred_data_blocks, vec_of_gammas);

    out.open ("/Users/daniilkolesnik/CLionProjects/AlgYaz/HW#1/uncyphred.txt");
    out << uncyphred_data;
    out.close();

    return 0;
}
