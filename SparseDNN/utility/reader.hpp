#pragma once

#include <iostream>
#include <numeric>
#include <algorithm>
#include <experimental/filesystem>
#include <fstream>
#include <Eigen/SparseCore>
#include <Eigen/Dense>
#include <vector>
#include <string>
#include <SparseDNN/utility/matrix_format.h>
#include <SparseDNN/utility/matrix_operation.hpp>

namespace std {
  namespace fs = experimental::filesystem;
}

namespace sparse_dnn {

template <typename T>
std::enable_if_t<std::is_same<T, float>::value, float> 
to_numeric(const std::string& str) {
  return std::stof(str);
}

template <typename T>
std::enable_if_t<std::is_same<T, double>::value, double> 
to_numeric(const std::string& str) {
  return std::stod(str);
}

template <typename T>
Eigen::SparseMatrix<T> tsv_string_to_matrix(
  const std::string& s,
  const int rows,
  const int cols,
  const int nnz
);

template <typename T>
void tsv_string_to_CSR_matrix(
  const std::string& s,
  const int rows,
  const int cols,
  const int nnz,
  CSRMatrix<T>& mat
);

template <typename T>
void tsv_string_to_CSC_matrix(
  const std::string& s,
  const int rows,
  const int cols,
  const int nnz,
  CSCMatrix<T>& mat
);

template <typename T>
void tsv_string_to_1D_array(
  const std::string& s,
  const int cols,
  T* arr
);

template <typename T>
void tsv_string_to_CSR_packed_array(
  const std::string& s,
  const int rows,
  const int cols,
  const int nnz,
  const int COL_BLK,
  const int N_SLAB,
  int* arr
);

template <typename T>
std::vector<Eigen::SparseMatrix<T> > read_weight(
  const std::fs::path& weight_dir,
  const int num_neurons_per_layer,
  const int num_layers
);

template <typename T>
void read_weight(
  const std::string& s,
  const int num_neurons_per_layer,
  const int nnz,
  CSRMatrix<T>& mat
);

template <typename T>
void read_weight(
  const std::fs::path& weight_dir,
  const int num_neurons_per_layer,
  const int max_nnz_per_layer,
  const int num_layers,
  const int COL_BLK,
  const int N_SLAB,
  const int pad,
  int* arr
);

template <typename T>
void read_weight_binary(
  const std::fs::path& weight_dir,
  const int num_neurons_per_layer,
  const int max_nnz_per_layer,
  const int num_layers,
  const int N_SLAB,
  const int pad,
  int* arr
);

template <typename T>
Eigen::SparseMatrix<T> read_input(
  const std::fs::path& input_path,
  const int num_inputs,
  const int num_features
);

template<typename T>
void read_input(
  const std::fs::path& input_path,
  const int num_inputs,
  const int num_featrues,
  T* arr,
  int* rlenY,
  int* rowsY,
  int& nerowsY
);

template<typename T>
void read_input(
  const std::fs::path& input_path,
  const int num_inputs,
  const int num_features,
  T* arr
);

template <typename T>
void read_input(
  const std::string& s,
  const int num_inputs,
  const int num_features,
  const int nnz,
  CSRMatrix<T>& mat
);

template <typename T>
void read_input(
  const std::fs::path& input_path,
  const int num_features,
  const int batch_size,
  T* arr
);

template <typename T>
void read_input_binary(
  const std::fs::path& input_path,
  T* arr,
  int* rlenY,
  int* rowsY,
  int& nerowsY
);

template <typename T>
void read_input_binary(
  const std::fs::path& input_path,
  const int batch_size,
  T* arr
);

template <typename T>
void read_input_binary(
  const std::fs::path& input_path,
  const int batch_size,
  T* arr,
  bool* rowsY
);

inline
Eigen::Matrix<int, Eigen::Dynamic, 1> read_golden(
  const std::fs::path& golden_path,
  const int num_inputs
);

inline
Eigen::Matrix<int, Eigen::Dynamic, 1> read_golden_binary(
  const std::fs::path& golden_path
);

inline
std::string read_file_to_string(const std::fs::path& path);

inline
void write_file_from_string(
  const std::fs::path& path,
  const std::string& s
);
    
inline
int find_max_nnz(
  const std::fs::path& weight_dir,
  const int num_layers,
  const int num_neurons_per_layer
);

inline
int find_max_nnz_binary(
  const std::fs::path& weight_dir,
  const int num_layers,
  const int num_neurons_per_layer
);

inline
int count_nnz(const std::string& s);

template <typename T>
void tsv_file_to_binary_file(
  const std::fs::path& weight_dir,
  const int num_layers,
  const int rows,
  const int cols,
  const int COL_BLK,
  const int N_SLAB,
  const int estimate_nnz
);

template <typename T>
void tsv_file_to_binary_file(
  std::fs::path file_path,
  const int rows,
  const int cols
);

template <typename T>
void tsv_file_to_binary_file(
  std::fs::path golden_path,
  const int num_features,
  const int num_layers,
  const int rows
);

//-----------------------------------------------------------------------------
//Definition of reader function
//-----------------------------------------------------------------------------

template <typename T>
Eigen::SparseMatrix<T> tsv_string_to_matrix(
  const std::string& s,
  const int rows,
  const int cols,
  const int nnz
) {
  //T is the floating posize_t type, either float or double
  static_assert(
    std::is_same<T, float>::value || std::is_same<T, double>::value,
    "data type must be either float or double"
  );

  typedef Eigen::Triplet<T> E;
  std::string line;
  std::vector<E> triplet_list;
  triplet_list.reserve(nnz);
  std::istringstream read_s(s);
  std::vector<std::string> tokens;

  while(std::getline(read_s, line)){
    std::istringstream lineStream(line);
    std::string token;
    tokens.clear();
    while(std::getline(lineStream, token, '\t')) {
      tokens.push_back(std::move(token));
    }
    triplet_list.emplace_back(
      std::stoi(tokens[0]) - 1,
      std::stoi(tokens[1]) - 1,
      to_numeric<T>(tokens[2])
    );
  }

  Eigen::SparseMatrix<T> mat(rows, cols);
  mat.reserve(triplet_list.size());
  mat.setFromTriplets(triplet_list.begin(), triplet_list.end());
  return mat;
}

template <typename T>
void tsv_string_to_CSR_matrix(
  const std::string& s,
  const int rows,
  const int cols,
  const int nnz,
  CSRMatrix<T>& mat
) {
  //T is the floating posize_t type, either float or double
  static_assert(
    std::is_same<T, float>::value || std::is_same<T, double>::value,
    "data type must be either float or double"
  );

  Eigen::SparseMatrix<T, Eigen::RowMajor> eigen_mat = tsv_string_to_matrix<T>(s, rows, cols, nnz);

  eigen_sparse_to_CSR_matrix<T>(eigen_mat, mat);
}

template <typename T>
void tsv_string_to_CSC_matrix(
  const std::string& s,
  const int rows,
  const int cols,
  const int nnz,
  CSCMatrix<T>& mat
) {
  //T is the floating posize_t type, either float or double
  static_assert(
    std::is_same<T, float>::value || std::is_same<T, double>::value,
    "data type must be either float or double"
  );
  auto eigen_mat = tsv_string_to_matrix<T>(s, rows, cols, nnz);
  eigen_sparse_to_CSC_matrix<T>(eigen_mat, mat);
}

//issue:doesnt't check boundary
template <typename T>
void tsv_string_to_1D_array(
  const std::string& s,
  const int cols,
  T* arr
) {
  //T is the floating posize_t type, either float or double
  static_assert(
    std::is_same<T, float>::value || std::is_same<T, double>::value,
    "data type must be either float or double"
  );

  std::string line;
  std::istringstream read_s(s);

  std::vector<std::string> tokens;

  while(std::getline(read_s, line)) {
    std::istringstream lineStream(line);
    std::string token;
    tokens.clear();
    while(std::getline(lineStream, token, '\t')) {
      tokens.push_back(std::move(token));
    }
    arr[(std::stoi(tokens[0]) - 1) * cols + std::stoi(tokens[1]) - 1] = to_numeric<T>(tokens[2]);
  }

}

template <typename T>
void tsv_string_to_CSR_packed_array(
  const std::string& s,
  const int rows,
  const int cols,
  const int nnz,
  const int COL_BLK,
  const int N_SLAB,
  int* arr
) {
  //T is the floating posize_t type, either float or double
  static_assert(
    std::is_same<T, float>::value || std::is_same<T, double>::value,
    "data type must be either float or double"
  );
  typedef Eigen::Triplet<T> E;
  std::string line;
  std::vector<E> triplet_list;
  triplet_list.reserve(nnz);
  std::istringstream read_s(s);
  std::vector<std::string> tokens;

  while(std::getline(read_s, line)) {
    std::istringstream lineStream(line);
    std::string token;
    tokens.clear();
    while(std::getline(lineStream, token, '\t')) {
      tokens.push_back(std::move(token));
    }
    triplet_list.emplace_back(
      std::stoi(tokens[0]) - 1 + 
      rows * ((std::stoi(tokens[1]) - 1) / COL_BLK),
      std::stoi(tokens[1]) - 1,
      to_numeric<T>(tokens[2])
    );
  }

  Eigen::SparseMatrix<T, Eigen::RowMajor> eigen_mat(rows * N_SLAB, cols);
  eigen_mat.reserve(triplet_list.size());
  eigen_mat.setFromTriplets(triplet_list.begin(), triplet_list.end());

  std::copy(eigen_mat.outerIndexPtr(), eigen_mat.outerIndexPtr() + rows * N_SLAB + 1, arr);
  std::copy(eigen_mat.innerIndexPtr(), eigen_mat.innerIndexPtr() + nnz, arr + rows * N_SLAB + 1);

  T* tmp = reinterpret_cast<T*>(arr + rows * N_SLAB + 1 + nnz);
  std::copy(eigen_mat.valuePtr(), eigen_mat.valuePtr() + nnz, tmp);
}

template <typename T>
std::vector<Eigen::SparseMatrix<T> > read_weight(
  const std::fs::path& weight_dir,
  const int num_neurons_per_layer,
  const int num_layers
) {

  //T is the floating posize_t type, either float or double
  static_assert(
    std::is_same<T, float>::value || std::is_same<T, double>::value,
    "data type must be either float or double"
  );

  std::vector<Eigen::SparseMatrix<T> > mats;
  mats.reserve(num_layers);

  //read weights
  for(size_t i = 0; i < num_layers; ++i) {
    std::fs::path p = weight_dir;
    p /= "n" + std::to_string(num_neurons_per_layer) + "-l"
      + std::to_string(i + 1) + ".tsv";
    auto data_str = read_file_to_string(p);
    mats.push_back(
      tsv_string_to_matrix<T>(
        data_str,
        num_neurons_per_layer,
        num_neurons_per_layer,
        count_nnz(data_str)
      )
    );
}
  return mats;
}

template <typename T>
void read_weight(
  const std::string& s,
  const int num_neurons_per_layer,
  const int nnz,
  CSRMatrix<T>& mat
) {
  tsv_string_to_CSR_matrix<T>(s, num_neurons_per_layer, num_neurons_per_layer, nnz, mat);
}

template <typename T>
void read_weight(
  const std::fs::path& weight_dir,
  const int num_neurons_per_layer,
  const int max_nnz_per_layer,
  const int num_layers,
  const int COL_BLK,
  const int N_SLAB,
  const int pad,
  int* arr
) {
  //T is the floating posize_t type, either float or double
  static_assert(
    std::is_same<T, float>::value || std::is_same<T, double>::value,
    "data type must be either float or double"
  );

  for(int i = 0; i < num_layers; ++i) {
    std::fs::path p = weight_dir;
    p /= "n" + std::to_string(num_neurons_per_layer) + "-l"
      + std::to_string(i + 1) + ".tsv";
    auto data_str = read_file_to_string(p);

    tsv_string_to_CSR_packed_array<T>(
      data_str,
      num_neurons_per_layer, 
      num_neurons_per_layer,
      max_nnz_per_layer,
      COL_BLK,  
      N_SLAB,
      arr + i * (num_neurons_per_layer * N_SLAB + 1
        + max_nnz_per_layer + pad + (sizeof(T) / sizeof(int)) * max_nnz_per_layer)
    );

  }
}

template <typename T>
void read_weight_binary(
  const std::fs::path& weight_dir,
  const int num_neurons_per_layer,
  const int max_nnz_per_layer,
  const int num_layers,
  const int N_SLAB,
  const int pad,
  int* arr
) {
  //T is the floating posize_t type, either float or double
  static_assert(
    std::is_same<T, float>::value || std::is_same<T, double>::value,
    "data type must be either float or double"
  );

  for(int i = 0; i < num_layers; ++i){
    std::fs::path p = weight_dir;
    p /= "n" + std::to_string(num_neurons_per_layer) + "-l"
      + std::to_string(i + 1) + ".b";
    std::ifstream in(p, std::ios::in | std::ios::binary);

    int rows;
    int nnz;
    int* location = arr + i * (num_neurons_per_layer * N_SLAB + 1
      + max_nnz_per_layer + (sizeof(T) / sizeof(int)) * max_nnz_per_layer + pad);

    in.read((char*)&rows, sizeof(int));
    in.read((char*)&nnz, sizeof(int));
    in.read((char*)location, sizeof(int) * (rows * N_SLAB + 1 + nnz) + sizeof(T) * nnz);
  }

}

template<typename T>
Eigen::SparseMatrix<T> read_input(
  const std::fs::path& input_path,
  const int num_inputs,
  const int num_features
) {

  //T is the floating posize_t type, either float or double
  static_assert(
    std::is_same<T, float>::value || std::is_same<T, double>::value,
    "data type must be either float or double"
  );

  auto input_str = read_file_to_string(input_path);
  return tsv_string_to_matrix<T>(
           input_str,
           num_inputs,
           num_features,
           count_nnz(input_str)
         );
}

template<typename T>
void read_input(
  const std::fs::path& input_path,
  const int num_inputs,
  const int num_features,
  T* arr,
  int* rlenY,
  int* rowsY,
  int& nerowsY
) {
  //T is the floating posize_t type, either float or double
  static_assert(
    std::is_same<T, float>::value || std::is_same<T, double>::value,
    "data type must be either float or double"
  );

  auto input_str = read_file_to_string(input_path);
  tsv_string_to_1D_array<T>(input_str, num_features, arr);

  nerowsY = 0;
  for(int i = 0; i < num_inputs; ++i) {
    rlenY[i] = std::count_if(
                 arr + i * num_features,
                 arr + (i + 1) * num_features,
                 [](T v){ return v != 0;}
               );
    rowsY[nerowsY++] = i;
  }
}

template<typename T>
void read_input(
  const std::fs::path& input_path,
  const int num_inputs,
  const int num_features,
  T* arr
) {
  //T is the floating posize_t type, either float or double
  static_assert(
    std::is_same<T, float>::value || std::is_same<T, double>::value,
    "data type must be either float or double"
  );

  auto input_str = read_file_to_string(input_path);
  tsv_string_to_1D_array<T>(input_str, num_features, arr);
}

template <typename T>
void read_input(
  const std::string& s,
  const int num_inputs,
  const int num_features,
  const int nnz,
  CSRMatrix<T>& mat
) {
  //T is the floating posize_t type, either float or double
  static_assert(
    std::is_same<T, float>::value || std::is_same<T, double>::value,
    "data type must be either float or double"
  );

  tsv_string_to_CSR_matrix<T>(s, num_inputs, num_features, nnz, mat);
}


template <typename T>
void read_input_binary(
  const std::fs::path& input_path,
  T* arr,
  int* rlenY,
  int* rowsY,
  int& nerowsY
) {
  //T is the floating posize_t type, either float or double
  static_assert(
    std::is_same<T, float>::value || std::is_same<T, double>::value,
    "data type must be either float or double"
  );

  std::fs::path p = input_path;
  std::ifstream in(p, std::ios::in | std::ios::binary);
  int num_inputs;
  int num_features;
  in.read((char*)&num_inputs, sizeof(int));
  in.read((char*)&num_features, sizeof(int));
  in.read((char*)arr, sizeof(T) * num_inputs * num_features);

  nerowsY = 0;
  for(int i = 0; i < num_inputs; ++i) {
    rlenY[i] = std::count_if(
                 arr + i * num_features,
                 arr + (i + 1) * num_features,
                 [](T v){ return v != 0;}
               );
    rowsY[nerowsY++] = i;
  }
}
template <typename T>
void read_input_binary(
  const std::fs::path& input_path,
  const int batch_size,
  T* arr
) {
  //T is the floating posize_t type, either float or double
  static_assert(
    std::is_same<T, float>::value || std::is_same<T, double>::value,
    "data type must be either float or double"
  );

  std::fs::path p = input_path;
  std::ifstream in(p, std::ios::in | std::ios::binary);
  int num_inputs;
  int num_features;
  in.read((char*)&num_inputs, sizeof(int));
  in.read((char*)&num_features, sizeof(int));
  in.read((char*)arr, sizeof(T) * num_inputs * num_features);
}

template <typename T>
void read_input_binary(
  const std::fs::path& input_path,
  const int batch_size,
  T* arr,
  bool* rowsY
) {
  //T is the floating posize_t type, either float or double
  static_assert(
    std::is_same<T, float>::value || std::is_same<T, double>::value,
    "data type must be either float or double"
  );

  std::fs::path p = input_path;
  std::ifstream in(p, std::ios::in | std::ios::binary);
  int num_inputs;
  int num_features;
  in.read((char*)&num_inputs, sizeof(int));
  in.read((char*)&num_features, sizeof(int));
  in.read((char*)arr, sizeof(T) * num_inputs * num_features);

  for(int i = 0; i < batch_size; ++i) {
    auto it  = std::find_if(
                 arr + i * num_features,
                 arr + (i + 1) * num_features,
                 [](T v){ return v != 0;}
               );
    rowsY[i] = (it != arr + (i + 1) * num_features) ? true : false;
  }
}

inline
Eigen::Matrix<int, Eigen::Dynamic, 1> read_golden(
  const std::fs::path& golden_path,
  const int num_inputs
) {
  std::string line;
  std::istringstream read_s{read_file_to_string(golden_path)};
  Eigen::Matrix<int, Eigen::Dynamic, 1> golden = Eigen::Matrix<int, Eigen::Dynamic, 1>::Zero(num_inputs, 1);

  while(std::getline(read_s, line)) {
    golden(std::stoi(line) - 1, 0) = 1;
  }   
  return golden;
}

inline
Eigen::Matrix<int, Eigen::Dynamic, 1> read_golden_binary(
  const std::fs::path& golden_path
) {
  std::ifstream in(golden_path, std::ios::in | std::ios::binary);

  int rows;
  in.read((char*)&rows, sizeof(int));

  Eigen::Matrix<int, Eigen::Dynamic, 1> golden(rows, 1);
  in.read((char*)golden.data(), sizeof(Eigen::Matrix<int, Eigen::Dynamic, 1>::Scalar) * rows);
  return golden;
}

inline
std::string read_file_to_string(const std::fs::path& path) {
  
  using namespace std::literals::string_literals;

  std::ifstream f{ path };

  if(!f) {
    throw std::runtime_error("cannot open the file"s + path.c_str());
  }

  std::stringstream sstream;
  sstream << f.rdbuf();
  return sstream.str();
}

inline
void write_file_from_string(
  const std::fs::path& path,
  const std::string& s
) {
  using namespace std::literals::string_literals;

  std::ofstream f{ path };

  if(!f){
    throw std::runtime_error("cannot open the file"s + path.c_str());
  }

  f.write(&s[0], std::fs::file_size(path));
}

inline
int find_max_nnz(
  const std::fs::path& weight_dir,
  const int num_layers,
  const int num_neurons_per_layer
) {

  int max_nnz{0};
  for(int i = 0; i < num_layers; ++i) {
    std::fs::path p = weight_dir;
    p /= "n" + std::to_string(num_neurons_per_layer) + "-l"
      + std::to_string(i + 1) + ".tsv";
    auto data_str = read_file_to_string(p);
    max_nnz = std::max(max_nnz, count_nnz(data_str));
  }

  return max_nnz;
}

inline
int find_max_nnz_binary(
  const std::fs::path& weight_dir,
  const int num_layers,
  const int num_neurons_per_layer
) {
  int max_nnz{0};
  for(int i = 0; i < num_layers; ++i) {
    std::fs::path p = weight_dir;
    p /= "n" + std::to_string(num_neurons_per_layer) + "-l"
      + std::to_string(i + 1) + ".b";
    std::ifstream in(p, std::ios::in | std::ios::binary);

    int nnz;
    int rows;
    in.read((char*)&rows, sizeof(int));
    in.read((char*)&nnz, sizeof(int));
    max_nnz = std::max(max_nnz, nnz);
  }

  return max_nnz;
}
inline
int count_nnz(const std::string& s) {
  return std::count(s.begin(), s.end(), '\n');
}

template <typename T>
void tsv_file_to_binary_file(
  const std::fs::path& weight_dir,
  const int num_layers,
  const int rows,
  const int cols,
  const int COL_BLK,
  const int N_SLAB,
  const int estimate_nnz
) {
  //T is the floating posize_t type, either float or double
  static_assert(
    std::is_same<T, float>::value || std::is_same<T, double>::value,
    "data type must be either float or double"
  );

  std::vector<Triplet<T> > triplets;
  triplets.reserve(estimate_nnz);

  for(int i = 0; i < num_layers; ++i) {
    triplets.clear();
    std::fs::path p = weight_dir;
    p /= "n" + std::to_string(cols) + "-l"
      + std::to_string(i + 1) + ".tsv";
    auto data_str = read_file_to_string(p);
    std::istringstream read_s(data_str);
    std::vector<std::string> tokens;
    std::string line;

    while(std::getline(read_s, line)) {
      std::istringstream lineStream(line);
      std::string token;
      tokens.clear();
      while(std::getline(lineStream, token, '\t')) {
        tokens.push_back(std::move(token));
      }
      triplets.emplace_back(
        std::stoi(tokens[0]) - 1 + 
        rows * ((std::stoi(tokens[1]) - 1) / COL_BLK),
        std::stoi(tokens[1]) - 1,
        to_numeric<T>(tokens[2])
      );
    }

    std::sort(triplets.begin(), triplets.end());
    int nnz = triplets.size();

    int row_array[rows * N_SLAB + 1];
    int col_array[nnz];
    T data_array[nnz];
    
    std::memset(row_array, 0, sizeof(int) * (rows * N_SLAB + 1));
    
    for(int j = 0 ; j < nnz; ++j) {
      ++row_array[triplets[j].row + 1];
      col_array[j] = triplets[j].col;
      data_array[j] = triplets[j].value;
    }

    std::partial_sum(row_array, row_array + rows * N_SLAB + 1, row_array);

    std::fs::path output_file = weight_dir;
    output_file /= "n" + std::to_string(cols) + "-l"
      + std::to_string(i + 1) + ".b";

    std::ofstream out(output_file, std::ios::out | std::ios::binary);
    out.write((char*)&rows, sizeof(int));
    out.write((char*)&nnz, sizeof(int));
    out.write((char*)row_array, sizeof(int) * (rows * N_SLAB + 1));
    out.write((char*)col_array, sizeof(int) * (nnz));
    out.write((char*)data_array, sizeof(T) * (nnz));
  }

  
}

template <typename T>
void tsv_file_to_binary_file(
  std::fs::path input_path,
  const int rows,
  const int cols
) {
  //T is the floating posize_t type, either float or double
  static_assert(
    std::is_same<T, float>::value || std::is_same<T, double>::value,
    "data type must be either float or double"
  );

  input_path /= "sparse-images-" + std::to_string(cols) + ".tsv";
  auto data_str = read_file_to_string(input_path);

  T data_array[rows * cols] = {0};
  //auto data_array = std::make_unique<T[]>(rows * cols);
  //std::memset(data_array, 0, sizeof(T) * rows * cols);

  std::istringstream read_s(data_str);
  std::vector<std::string> tokens;
  std::string line;
  std::string token;

  while(std::getline(read_s, line)) {
    std::istringstream lineStream(line);
    tokens.clear();
    while(std::getline(lineStream, token, '\t')) {
      tokens.push_back(std::move(token));
    }
    *(data_array + (std::stoi(tokens[0]) - 1) * cols + std::stoi(tokens[1]) - 1) = to_numeric<T>(tokens[2]);
  }

  std::fs::path p = input_path.parent_path();
  p /= "sparse-images-" + std::to_string(cols) + ".b";

  std::ofstream out(p, std::ios::out | std::ios::binary);
  out.write((char*)&rows, sizeof(int));
  out.write((char*)&cols, sizeof(int));
  out.write((char*)data_array, sizeof(T) * (rows * cols));
}

template <typename T>
void tsv_file_to_binary_file(
  std::fs::path golden_path,
  const int num_features,
  const int num_layers,
  const int rows
) {
  //T is the floating posize_t type, either float or double
  static_assert(
    std::is_same<T, float>::value || std::is_same<T, double>::value,
    "data type must be either float or double"
  );

  std::string line;
  golden_path /= "neuron" + std::to_string(num_features) + "-l" + std::to_string(num_layers) + "-categories.tsv";
  std::istringstream read_s{read_file_to_string(golden_path)};

  Eigen::Matrix<int, Eigen::Dynamic, 1> golden = Eigen::Matrix<int, Eigen::Dynamic, 1>::Zero(rows, 1);

  while(std::getline(read_s, line)) {
    golden(std::stoi(line) - 1, 0) = 1;
  }   

  std::fs::path p = golden_path.parent_path();
  p /= "neuron" + std::to_string(num_features) + "-l" + std::to_string(num_layers) + "-categories.b";

  std::ofstream out(p, std::ios::out | std::ios::binary);

  out.write((char*)&rows, sizeof(int));
  out.write(
    (char*) golden.data(),
    sizeof(Eigen::Matrix<int, Eigen::Dynamic, 1>::Scalar) * rows
  );
  
}
} // end of namespace sparse_dnn-----------------------------------------------
