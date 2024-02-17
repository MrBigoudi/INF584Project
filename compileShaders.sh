#!/bin/bash

# Set the input and output directories
inputDirectory="shaders"
outputDirectory="shaders/compiled"
compiler="glslc"
types=("vert" "frag")

# Empty the output directory
rm -rf "${outputDirectory}"

# Create the output directory if it doesn't exist
mkdir -p "${outputDirectory}"

# Loop through all shader files in the input directory
for shaderType in "${types[@]}"; do
    for file in "${inputDirectory}"/*."${shaderType}"; do
        # Extract the base name of the file without extension
        baseName=$(basename "${file%.*}")
        # Set the output file name with the .spv extension
        outputFile="${outputDirectory}/${baseName}.${shaderType}.spv"

        # Compile the shader using glslc
        glslc "${file}" -o "${outputFile}"

        # Check if the compilation was successful
        if [ $? -eq 0 ]; then
            echo "Successfully compiled shader ${file} to ${outputFile}"
        else
            echo "Failed to compile shader ${file}"
        fi
    done
done