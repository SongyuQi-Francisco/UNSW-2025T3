# 2025T3 Course Assignments Repository

This repository contains course assignments and project code for Term 3, 2025 (2025T3).

## 📚 Course Directory

### COMP9319 - Data Compression and Indexing

#### Assignment 1 - LZW Compression Algorithm
- **Location**: `9319/Ass1/`
- **Content**: LZW encoding and decoding implementation
- **Main Files**:
  - `lencode.cpp` - LZW encoder
  - `ldecode.cpp` - LZW decoder
- **Features**: Implements LZW compression algorithm with variable-length encoding support (1-byte, 2-byte)

#### Assignment 2 - BWT Transform and Search
- **Location**: `9319/Ass2/`
- **Content**: Burrows-Wheeler Transform (BWT) encoding, decoding, and search
- **Main Files**:
  - `bwtdecode.cpp` - BWT decoder
  - `bwtsearch.cpp` - BWT searcher (supports DNA sequence search)
  - `bwt-util.cpp/h` - BWT utility functions
  - `a_kmp.cpp` - KMP string matching algorithm implementation
- **Features**: 
  - BWT encoding and decoding
  - Fast DNA sequence search based on BWT
  - Memory optimization using checkpoints

#### Final - Comprehensive Compression Project
- **Location**: `9319/Final/`
- **Content**: Comprehensive implementation of multiple compression algorithms
- **Main Files**:
  - `arithmetic_encode.cpp` - Arithmetic encoding
  - `arithmetic_decode.cpp` - Arithmetic decoding
  - `lzw-encode.cpp` - LZW encoding
  - `lzw-decode.cpp` - LZW decoding
  - `bwt-encode.cpp` - BWT encoding
  - `bwtdecode.cpp` - BWT decoding
  - `bwtsearch.cpp` - BWT search
- **Compilation**: Use `Makefile` to compile all programs

### COMP9517 - Computer Vision

#### Lab Assignments
- **Location**: `9517/Lab/`
- **Lab1**: Image processing fundamentals
- **Lab2**: Image feature extraction (`9517Lab2.ipynb`)
- **Lab3**: Large-scale image dataset processing (contains 15,000+ images)
- **Lab4**: Advanced computer vision techniques

#### Group Project
- **Location**: `9517/Group Project/`
- **Content**: Group project code and report
- **Files**: 
  - `9517code.zip` - Project source code
  - `Report.pdf` - Project report

#### Final
- **Location**: `9517/Final/`
- **Content**: Exam materials and sample questions

### COMP3411 - Artificial Intelligence

#### Assignment 1 - Neural Networks and Time Series Prediction
- **Location**: `9814/Ass1/`
- **Content**: El Nino index prediction using artificial neural networks
- **Main Files**:
  - `z5536858_assignment1.ipynb` - Main program (Jupyter Notebook)
  - `model_nino_tplus*.pth` - Trained model files (predictions for t+1 to t+6)
  - `model_taskB.pth` / `improved_model_taskB.pth` - Task B models
  - `feature_scaler.pkl` - Feature scaler
- **Features**:
  - Predicts future values using historical Nino3.4 data
  - Supports multi-step prediction (t+1 to t+6)
  - Includes improved models and transfer learning versions

#### Assignment 2 - Reinforcement Learning
- **Location**: `9814/Ass2/`
- **Content**: Q-learning reinforcement learning implementation
- **Main Files**:
  - `z5536858_assignment2.ipynb` - Main program
  - `safety_shield_model.pth` - Safety shield model
  - `task2_q_table_gentle.npy` - Q-table data
  - `task2_comparison_plots.png` - Result comparison plots
  - `complete_dataset.pkl` - Complete dataset

## 🛠️ Tech Stack

- **Programming Languages**: C++, Python, Jupyter Notebook
- **Deep Learning Framework**: PyTorch
- **Data Processing**: NumPy, Pandas, scikit-learn
- **Visualization**: Matplotlib
- **Build Tools**: Make, g++

## 📝 Compilation Instructions

### COMP9319 Projects

#### Assignment 2
```bash
cd 9319/Ass2/a2/
make
```

#### Final Project
```bash
cd 9319/Final/
make
```

Clean compiled files:
```bash
make clean
```

## 📊 Project Features

1. **Data Compression Algorithms**: Implementation of various classic compression algorithms (LZW, BWT, Arithmetic Coding)
2. **Efficient Search**: BWT search algorithm optimized for DNA sequences with checkpoint mechanism for memory efficiency
3. **Deep Learning**: Neural network implementation using PyTorch for time series prediction
4. **Reinforcement Learning**: Q-learning algorithm implementation for solving reinforcement learning problems
5. **Computer Vision**: Includes image processing, feature extraction, and other CV tasks

## 📄 License

This repository is for academic purposes only and contains course assignment and project code.

## ⚠️ Notes

- This repository contains course assignment code. Please adhere to academic integrity principles
- Some files are large (e.g., image datasets in Lab3) and are ignored using `.gitignore`
- Model files (`.pth`) and dataset files (`.pkl`, `.csv`) may be large
