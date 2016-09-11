#include "pointset.hpp"

namespace U3D
{

PointSet::PointSet(BitStreamReader& reader) : CLOD_Object(reader)
{
}

void PointSet::update_resolution(BitStreamReader& reader)
{
    reader.read<uint32_t>();    //Chain index is always zero.
    uint32_t start, end;
    reader >> start >> end;
    for(unsigned int resolution = start; resolution < end; resolution++) {
        uint32_t split_position, split_point;
        Vector3f new_position;
        if(resolution == 0) {
            split_position = reader[cZero].read<uint32_t>();
        } else {
            split_position = reader[resolution].read<uint32_t>();
            split_point = indexer.get_point(split_position);
            new_position = positions[points[split_point].position];
        }
        uint8_t pos_sign = reader[cPosDiffSign].read<uint8_t>();
        uint32_t pos_X = reader[cPosDiffX].read<uint32_t>();
        uint32_t pos_Y = reader[cPosDiffY].read<uint32_t>();
        uint32_t pos_Z = reader[cPosDiffZ].read<uint32_t>();
        new_position += Vector3f::dequantize(pos_sign, pos_X, pos_Y, pos_Z, position_iq);
        positions.push_back(new_position);
        uint32_t new_normal_count = reader[cNormalCnt].read<uint32_t>();
        Vector3f pred_normal;
        if(resolution > 0) {
            pred_normal = normals[points[split_point].normal];
        }
        for(unsigned int i = 0; i < new_normal_count; i++) {
            uint8_t norm_sign = reader[cDiffNormalSign].read<uint8_t>();
            uint32_t norm_X = reader[cDiffNormalX].read<uint32_t>();
            uint32_t norm_Y = reader[cDiffNormalY].read<uint32_t>();
            uint32_t norm_Z = reader[cDiffNormalZ].read<uint32_t>();
            normals.push_back(pred_normal + Vector3f::dequantize(norm_sign, norm_X, norm_Y, norm_Z, normal_iq));
        }
        uint32_t new_point_count = reader[cPointCnt].read<uint32_t>();
        for(unsigned int i = 0; i < new_point_count; i++) {
            Point new_point;
            new_point.shading_id = reader[cShading].read<uint32_t>();
            new_point.normal = normals.size() - new_normal_count + reader[cNormalIdx].read<uint32_t>();
            if(shading_descs[new_point.shading_id].attributes & 0x00000001) {
                uint8_t dup_flag = reader[cDiffDup].read<uint8_t>();
                if(!(dup_flag & 0x2)) {
                    uint8_t diffuse_sign = reader[cDiffuseColorSign].read<uint8_t>();
                    uint32_t diffuse_R = reader[cColorDiffR].read<uint32_t>();
                    uint32_t diffuse_G = reader[cColorDiffG].read<uint32_t>();
                    uint32_t diffuse_B = reader[cColorDiffB].read<uint32_t>();
                    uint32_t diffuse_A = reader[cColorDiffA].read<uint32_t>();
                    
                    new_point.diffuse = diffuse_colors.size();
                    diffuse_colors.push_back(Color4f::dequantize(diffuse_sign, diffuse_R, diffuse_G, diffuse_B, diffuse_A, diffuse_iq));
                } else {
                    new_point.diffuse = last_diffuse;
                }
                last_diffuse = new_point.diffuse;
            }
            if(shading_descs[new_point.shading_id].attributes & 0x00000002) {
                uint8_t dup_flag = reader[cSpecDup].read<uint8_t>();
                if(!(dup_flag & 0x2)) {
                    uint8_t specular_sign = reader[cSpecularColorSign].read<uint8_t>();
                    uint32_t specular_R = reader[cColorDiffR].read<uint32_t>();
                    uint32_t specular_G = reader[cColorDiffG].read<uint32_t>();
                    uint32_t specular_B = reader[cColorDiffB].read<uint32_t>();
                    uint32_t specular_A = reader[cColorDiffA].read<uint32_t>();
                    
                    new_point.specular = specular_colors.size();
                    specular_colors.push_back(Color4f::dequantize(specular_sign, specular_R, specular_G, specular_B, specular_A, specular_iq));
                } else {
                    new_point.specular = last_specular;
                }
                last_specular = new_point.specular;
            }
            for(unsigned int j = 0; j < get_texlayer_count(new_point.shading_id); j++) {
                uint8_t dup_flag = reader[cTexCDup].read<uint8_t>();
                if(!(dup_flag & 0x2)) {
                    uint8_t texcoord_sign = reader[cTexCoordSign].read<uint8_t>();
                    uint32_t texcoord_U = reader[cTexCDiffU].read<uint32_t>();
                    uint32_t texcoord_V = reader[cTexCDiffV].read<uint32_t>();
                    uint32_t texcoord_S = reader[cTexCDiffS].read<uint32_t>();
                    uint32_t texcoord_T = reader[cTexCDiffT].read<uint32_t>();

                    new_point.texcoord[j] = texcoords.size();
                    texcoords.push_back(TexCoord4f::dequantize(texcoord_sign, texcoord_U, texcoord_V, texcoord_S, texcoord_T, texcoord_iq));
                } else {
                    new_point.texcoord[j] = last_texcoord;
                }
                last_texcoord = new_point.texcoord[j];
            }
            points.push_back(new_point);
        }
    }
}

}
