#include "step_generated.h"
#include "flatbuffers/util.h"
#include <iostream>
#include <memory>
#include <string>
#include <fstream>
#include <new>
#include <cstdlib>
#include <stdio.h>

static uint32_t alloc;
static uint32_t array_alloc;
static uint32_t deletes;
static uint32_t array_deletes;

// Summary of allocations when using flatbuffers:
void* operator new  ( std::size_t count )
{
  ++alloc;
  return std::malloc(count);
}

void* operator new[]( std::size_t count )
{
  ++array_alloc;
  return std::malloc(count);
}

void operator delete  ( void* ptr )
{
  ++deletes;
  std::free(ptr);
}

void operator delete[]( void* ptr ) 
{
  ++array_deletes;
  std::free(ptr);
}

void print_stats()
{
  printf("%d alloc %d array alloc %d deletes %d array deletes\n", 
      alloc,
      array_alloc,
      deletes,
      array_deletes);
}

// Memorize these steps:
// Create a root structure
// use rvalue set on any unions contained in the structure
// Any types that contain structures require pointer management
// access any unions contained in the structure using As...() methods
// call Pack function (static class member of the root table type)
// pass the pointer from Pack() to a FlatBufferBuilder::Finish...
//
// Decode:
// Root types have a Verify... function for untrusted binary sources
// Root types have a Get... function to access a verified buffer
// tables types may be accessed using UnPackTo()
// union types are accessed using As...()
int main(int argc, char** )
{
    print_stats();
    printf("Hello\n");

    flatbuffers::FlatBufferBuilder b;
    printf("FlatBufferBuilder created\n");
    print_stats();

    fbs::v3i temp(3, 4, 5);
    flatbuffers::Offset<fbs::SpawnStep> off = fbs::CreateSpawnStep(b, 1, &temp, 0);
    flatbuffers::Offset<fbs::AnyStep> any = fbs::CreateAnyStep(b, fbs::StepUnion_SpawnStep, off.Union());
    FinishAnyStepBuffer(b, any);

    printf("Traditional CreateSpawnStep call\n");
    print_stats();

    // This block is a great example of using raw struct access
    // Note: You pay an extra allocation
    // The struct has already performed endian conversions etc
    // Whereas the buffer builder + tables perform such conversions
    // 
    //fbs::AnyStepT root;
    //printf("AnyStepT created on stack\n");
    //print_stats();
    //root.step.Set(fbs::SpawnStepT());
    //printf("Constructed SpawnStepT() in AnyStepT\n");
    //print_stats();

    //fbs::SpawnStepT& instance = *root.step.AsSpawnStep();
    //printf("SpawnStepT requested from AnyStepT\n");
    //instance.player = 0;
    //instance.unit_type = 1;
    //print_stats();
    //instance.location.mutate_x(3);
    //instance.location.mutate_y(4);
    //instance.location.mutate_z(5);
    //printf("Set values for v3i\n");

    //fbs::FinishAnyStepBuffer(b, fbs::AnyStep::Pack(b, &root));
    printf("Root step size %d\n", b.GetSize());
    print_stats();
    
    {
      flatbuffers::FlatBufferBuilder b2;
      printf("Throw away fbb :)\n");
      print_stats();
    }

    if (argc > 1)
    {
        printf("Writing fresh output to dump.bin\n");
        std::ofstream output("dump.bin", std::ios::binary);
        output.write((const char*)b.GetBufferPointer(), b.GetSize());
        output.flush();
    }
    print_stats();

    for (int i = 0; i < 5; ++i)
    {
      getchar();
      printf("Begin read cycle\n");
      print_stats();
      std::string buffer;
      flatbuffers::LoadFile("dump.bin", true, &buffer);
      printf("File read completed, %d bytes.\n", (unsigned int)buffer.size());
      print_stats();

      flatbuffers::Verifier verifier(reinterpret_cast<const uint8_t*>(buffer.c_str()), buffer.length());
      printf("Verifier checking %ul bytes\n",(unsigned int) buffer.length());
      if(!fbs::VerifyAnyStepBuffer(verifier)) 
      {
          std::cout << "Invalid binary read from dump.bin" << std::endl;
          return 1;
      }
      printf("Verifier completed.\n");
      print_stats();

      printf("Begin unpack\n");
      auto unpackRoot = fbs::GetAnyStep(buffer.c_str());
      const fbs::SpawnStep* foo = unpackRoot->step_as_SpawnStep();
      print_stats();
      //fbs::AnyStepT unpackStep;
      //unpackRoot->UnPackTo(&unpackStep);
      //printf("AnyStepT unpacked\n");
      //print_stats();
      //fbs::SpawnStepT& unpackSpawn = *unpackStep.step.AsSpawnStep();
      //printf("SpawnStepT unpacked\n");
      //print_stats();
      std::cout << foo->player() << " "
          << foo->unit_type() << " ";
      std::cout << foo->location()->x() << " "
          << foo->location()->y() << " "
          << foo->location()->z() << " "
          << std::endl;
      print_stats();
    }
}

