
#include <yocto_extension/yocto_extension.h>
#include <yocto/yocto_math.h>
#include <yocto/yocto_commonio.h>
#include <yocto/ext/json.hpp>
#include <fstream>
using namespace yocto::math;
using namespace yocto::commonio;
using json = nlohmann::json;


int main(int argc, const char* argv[])
{
    auto cli = make_cli("treegenerator", "Space Colonization trees generator");

    





  
  // ############# PARAMETERS #############
  // SHAPE PARAMETERS
      // Number of points to be included in the general shape
      int n_attractors = 2000;
      // Dimensions of the cube in which n_points attractors will be generated
      int attractors_range_min = -20;
      int attractors_range_max = 20;
      // Offset in the Z direction of the initial cube of points
      int attractors_z_offset = 1;
      // Type of function to be used to generate the cloud
      int f_selector = 1;
  // TREE PARAMETERS
      // Point in wich the tree will start to grow
      vec3f tree_starting_point = vec3f{0, 0, 0};
      // Approximated point in wich the trunk will start to grow branches (trunk position)
      vec3f trunk_length = vec3f{0, 0, 1};
      // Max number of tree nodes
      int max_nodes = 5000; 
      // Length of a generic branch
      float D = 0.2;
      // Min width of a branch
      float W = 0.005;
      // Max width of a branch, 50 is a good value (not in the same scale of W, it's just a constant that will be multiplied by W)
      float max_width = 50;
      // Attractor inluence shpere
      float max_influence_sphere = 5*D;
      // Attractor kill distance from a branch
      float max_killing_radius = 3.5*D;
      //Eventual tropism (deformation or gravity) of a branch
      vec3f tropism = vec3f{0, 0, 0};
  // LEAVES PARAMETERS
      // Min number of leaves in a single branch of lenght D
      int leaves_density_min = 8;
      // Max number of leaves in a single branch of length D
      int leaves_density_max = 15;
      // Max size of a leaf
      float leaf_size_max = 0.1;
      // Min size of a leaf
      float leaf_size_min = 0.05;
      // The following are 4 parameters of the logaritmic spiral used for distributing the leaves. 
      float a_spiral = 1;
      float k_spiral = 2;
      float e = 2.7;
      float rounds = 9;
  // Texture parameters
      // Number of differents leaves
      int leaves_textures_number = 3;
      // For the texture, if you are using single object mode, don't worry. 
      // If you are using the multiple object mode, call the leaf texture with a index that start from 0 at the end.
      // For example:
          // leaf_texture_0.png
          // leaf_opacity_texture_0.png
          // leaf_texture_1.png
          // leaf_opacity_texture_1.png
      // And pass the path WITHOUT numbers and extension (hopefully a png)
          // leaf_texture_
          // leaf_opacity_texture_
      // Single object mode texture
      std::string single_mode_texture_path = "resources/exports/tree.png";
      std::string single_mode_opacity_texture_path = "resources/exports/tree_opacity.png";
      // Multiple object mode textures
      std::string multiple_mode_tree_texture_path = "resources/exports/wood.png";
      std::string multiple_mode_leaf_texture_path = "resources/exports/leaf_";
      std::string multiple_mode_leaf_opacity_texture_path = "resources/exports/leaf_opacity_";
      //Remember to have a shapes and textures folder!
      std::string export_name_path = "resources/exports/albero.json";
      int single_object = 1;
      //Load configuration from JSON file
      std::string configuration_path = "";

  // ######################################
    float trunk_z = 1;

    add_option(cli, "--json,-j", configuration_path, "JSON configuration file path");
    add_option(cli, "-o", export_name_path, "Output path");
    add_option(cli, "--node,-n", n_attractors, "Nodes number");
    add_option(cli, "--f-generator,-f", f_selector, "0 for a sphere, 1 for a rotational solid, 2 for a cone");
    add_option(cli, "--attractors_min,-m", attractors_range_min, "Attractors minium number");
    add_option(cli, "--attractors_max,-M", attractors_range_max, "Attractors max number");
    add_option(cli, "--z-offset,-z", attractors_z_offset, "Z offset of the cloud");
    add_option(cli, "--trunk-length", trunk_z, "Lenght of the trunk");
    add_option(cli, "--max-branches,-b", max_nodes, "Max number of tree nodes");
    add_option(cli, "--branches-length,-l", D, "Length of a generic branch");
    add_option(cli, "--influence-sphere,-d", max_influence_sphere, "Max distance for an attractor to work");
    add_option(cli, "--killing-radius,-k", max_influence_sphere, "Min distance for an attractor from a branch");
    add_option(cli, "--single-object,-s", single_object, "Single object mode");
    add_option(cli, "--m-trunk-texture",  multiple_mode_tree_texture_path, "Multiple mode trunk texture path");
    add_option(cli, "--m-leaves-textures",  multiple_mode_leaf_texture_path, "Multiple mode leaves texture path");
    add_option(cli, "--m-leaves-textures-opacity",  multiple_mode_leaf_opacity_texture_path, "Multiple mode leaves opacity_texture path");
    add_option(cli, "--s-textures",  single_mode_texture_path, "Single mode texture");
    add_option(cli, "--s-textures-opacity",  single_mode_opacity_texture_path, "Single mode opacity_texture");
    add_option(cli, "--leaves-textures,-t", leaves_textures_number, "Number of differents textures for leaves");
    add_option(cli, "--leaves-max-dim", leaf_size_max, "Max size of a leaf, 0 for no leaves on the tree");
    

    parse_cli(cli, argc, argv);
    
    trunk_length.z = trunk_z;

    if(configuration_path != "")
    {
        std::ifstream fs(configuration_path);
        json j;
        fs >> j;
        std::cout << "Loading configuration file... \n";
        n_attractors = j["n_attractors"];
        attractors_range_min = j["attractors_range_min"];
        attractors_range_max = j["attractors_range_max"];
        attractors_z_offset = j["attractors_z_offset"];
        f_selector = j["f_selector"];
        tree_starting_point = vec3f{j["tree_starting_point"][0], j["tree_starting_point"][1], j["tree_starting_point"][2]} ;
        trunk_length = vec3f{j["trunk_length"][0],j["trunk_length"][1],j["trunk_length"][2]};
        max_nodes = j["max_nodes"];
        D = j["D"];
        W = j["W"];
        max_width = j["max_width"];
        max_influence_sphere = j["max_influence_sphere"];
        max_killing_radius  = j["max_killing_radius"];
        tropism = vec3f{j["tropism"][0], j["tropism"][1], j["tropism"][2]} ;
        leaves_density_max = j["leaves_density_max"];
        leaves_density_min = j["leaves_density_min"];
        leaf_size_max = j["leaf_size_max"];
        leaf_size_min = j["leaf_size_min"];
        a_spiral = j["a_spiral"];
        k_spiral = j["k_spiral"];
        rounds = j["rounds"];
        leaves_textures_number = j["leaves_textures_number"];
        single_mode_texture_path = j["single_mode_texture_path"];
        single_mode_opacity_texture_path = j["single_mode_opacity_texture_path"];
        multiple_mode_tree_texture_path = j["multiple_mode_tree_texture_path"];
        multiple_mode_leaf_texture_path = j["multiple_mode_leaf_texture_path"];
        multiple_mode_leaf_opacity_texture_path = j["multiple_mode_leaf_opacity_texture_path"];
        single_object = j["single_object"];
        std::cout << j.dump() << "\n";
    }   
    
    
    
    yocto::extension::generate_tree(n_attractors, attractors_range_min, attractors_range_max, attractors_z_offset,
        tree_starting_point, trunk_length, max_nodes, D, W, max_width,
        max_influence_sphere, max_killing_radius, tropism, leaves_density_min, leaves_density_max,
        leaf_size_max, leaf_size_min, a_spiral, k_spiral, e, rounds, leaves_textures_number,
        single_object, single_mode_texture_path, single_mode_opacity_texture_path,
        multiple_mode_tree_texture_path, multiple_mode_leaf_texture_path, 
        multiple_mode_leaf_opacity_texture_path, export_name_path, f_selector);

    return 0;
}