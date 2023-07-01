#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include <pugixml/pugixml.hpp>

#include <kwaslib/kwas_all.h>

void load_BINA_save_XML(const char* file);

int main(int argc, char** argv)
{
    if(argc == 1)
    {
        printf("Usage: %s file.xml/.pcmodel\n", argv[0]);
        return 0;
    }
    
    const uint32_t path_len = strlen(argv[1]);
    
    if(pu_is_file(argv[1]))
    {
        if(strncmp(&argv[1][path_len-4], ".xml", 4) == 0)
        {
            printf("No support for XML yet\n");
        }
        else
        {
            load_BINA_save_XML(argv[1]);
        }
    }
    else
    {
        printf("Input is not a file.\n");
    }
    
    return 0;
}

void load_BINA_save_XML(const char* file)
{
    pugi::xml_document doc;

    uint8_t* file_data = (uint8_t*)calloc(1, 0x10);
    FILE* f = fopen(file, "rb");
    if(fread(file_data, 0x10, 1, f) == 0)
    {
        printf("Could not read data from a file\n");
        free(file_data);
        fclose(f);
        return;
    }
    fseek(f, 0, SEEK_SET);

    BINA_Header header = {0};
    if(BINA_read_header(file_data, &header) == 0)
    {
        printf("File does not include a BINA header.\n");
        free(file_data);
        fclose(f);
        return;
    }
    
    printf("%.4s %c %c %c %c %u %hu %hu\n",
            header.magic, header.ver_major, header.ver_minor,
            header.ver_rev, header.endianness, header.file_size, 
            header.node_count, header.unk_01);

    pugi::xml_node BINA_xml_node = doc.append_child("BINA");
    BINA_xml_node.append_attribute("ver_major").set_value((const pugi::char_t*)&header.ver_major, 1);
    BINA_xml_node.append_attribute("ver_minor").set_value((const pugi::char_t*)&header.ver_minor, 1);
    BINA_xml_node.append_attribute("ver_rev").set_value((const pugi::char_t*)&header.ver_rev, 1);
    BINA_xml_node.append_attribute("endianness").set_value((const pugi::char_t*)&header.endianness, 1);

    free(file_data);
    file_data = (uint8_t*)calloc(1, header.file_size);
    if(fread(file_data, 1, header.file_size, f) == 0)
    {
        printf("Could not read data from a file.\n");
        free(file_data);
        fclose(f);
        return;
    }
    fclose(f);
    
    BINA_DATA_Chunk data_chunk = {0};
    BINA_read_DATA(file_data, &data_chunk);
    printf("%.4s %u %u %u %u %u\n",
            data_chunk.magic, data_chunk.data_size, data_chunk.string_table_offset,
            data_chunk.string_table_size, data_chunk.offset_table_size,
            data_chunk.additional_data_size);

    pugi::xml_node DATA_xml_node = BINA_xml_node.append_child("DATA");

    /*for(uint32_t i = 0; i < data_chunk.offsets_size; ++i)
    {
        printf("Offset[%04u] Flag: %u | Offset Value: 0x%04x | Pos: 0x%04x | Value: 0x%04x\n", 
                i, data_chunk.offsets[i].off_flag, data_chunk.offsets[i].off_value,
                data_chunk.offsets[i].pos, data_chunk.offsets[i].value);
    }*/

    BINA_CPIC_Chunk cpic_chunk = {0};
    BINA_read_CPIC(file_data, &cpic_chunk);
    printf("%.4s %u 0x%x %u %u %u\n",
            cpic_chunk.magic, cpic_chunk.ver_number, cpic_chunk.instances_offset,
            cpic_chunk.unk_01, cpic_chunk.instances_count, cpic_chunk.unk_02);
    
    pugi::xml_node CPIC_xml_node = DATA_xml_node.append_child("CPIC");
    CPIC_xml_node.append_attribute("ver_number").set_value(cpic_chunk.ver_number);

    for(uint32_t i = 0; i < cpic_chunk.instances_count; ++i)
    {
        /*printf("Instance[%04u] %s\n", i, &file_data[0x40 + cpic_chunk.instances[i].name_offset]);
        printf("\tPos: {%f, %f, %f}\n", cpic_chunk.instances[i].pos_x,
                                        cpic_chunk.instances[i].pos_y,
                                        cpic_chunk.instances[i].pos_z);
        printf("\tRot: {%f, %f, %f}\n", cpic_chunk.instances[i].rot_x,
                                        cpic_chunk.instances[i].rot_y,
                                        cpic_chunk.instances[i].rot_z);
        printf("\tScale: {%f, %f, %f}\n", cpic_chunk.instances[i].scale_x,
                                          cpic_chunk.instances[i].scale_y,
                                          cpic_chunk.instances[i].scale_z);*/

        pugi::xml_node cur_instance = CPIC_xml_node.append_child("cpic_instance");
        cur_instance.append_attribute("name").set_value((const pugi::char_t*)&file_data[0x40 + cpic_chunk.instances[i].name_offset]);
        cur_instance.append_attribute("name2").set_value((const pugi::char_t*)&file_data[0x40 + cpic_chunk.instances[i].name2_offset]);
        
        cur_instance.append_child("pos").append_attribute("x").set_value(cpic_chunk.instances[i].pos_x, 6);
        cur_instance.child("pos").append_attribute("y").set_value(cpic_chunk.instances[i].pos_y, 6);
        cur_instance.child("pos").append_attribute("z").set_value(cpic_chunk.instances[i].pos_z, 6);
        
        cur_instance.append_child("rot").append_attribute("x").set_value(cpic_chunk.instances[i].rot_x, 6);
        cur_instance.child("rot").append_attribute("y").set_value(cpic_chunk.instances[i].rot_y, 6);
        cur_instance.child("rot").append_attribute("z").set_value(cpic_chunk.instances[i].rot_z, 6);
        
        cur_instance.append_child("scale").append_attribute("x").set_value(cpic_chunk.instances[i].scale_x, 6);
        cur_instance.child("scale").append_attribute("y").set_value(cpic_chunk.instances[i].scale_y, 6);
        cur_instance.child("scale").append_attribute("z").set_value(cpic_chunk.instances[i].scale_z, 6);
    }
    
    const uint32_t filename_len = strlen(file);
    char* filename_xml = (char*)calloc(filename_len+5, 1);
    memcpy(filename_xml, file, filename_len);
    strncpy(&filename_xml[filename_len], ".xml\0", 5);
    
    printf("%s\n", filename_xml);
    
    doc.save_file(filename_xml);

    free(cpic_chunk.instances);
    free(data_chunk.offsets);
}