<?php

function tripal_phylogeny_prepare_tree_viewer($phylotree) {

  // Don't prepare for viewing more than once.
  if (property_exists($phylotree, 'prepared_to_view') and
      $phylotree->prepared_to_view == TRUE) {
    return;
  }

  $module_path = drupal_get_path('module', 'tripal_phylogeny');

  drupal_add_js('https://d3js.org/d3.v3.min.js', 'external');

  drupal_add_js("$module_path/theme/js/d3.phylogram.js");
  drupal_add_js("$module_path/theme/js/tripal_phylogeny.js");
  drupal_add_css("$module_path/theme/css/tripal_phylogeny.css");

  drupal_add_library('system', 'ui.dialog');

  // Don't show tick marks for the taxonomy tree.
  $skip_ticks = 0;
  if ($phylotree->type_id->name == 'taxonomy') {
    $skip_ticks = 1;
  }

  // Get the tree options as set by the administrator.
  $options = json_encode(array(
    'phylogram_width' => variable_get('tripal_phylogeny_default_phylogram_width', 350),
    'root_node_size' => variable_get('tripal_phylogeny_default_root_node_size', 3),
    'interior_node_size' => variable_get('tripal_phylogeny_default_interior_node_size', 0),
    'leaf_node_size' => variable_get('tripal_phylogeny_default_leaf_node_size', 6),
    'skipTicks' => $skip_ticks,
  ));

  // Get the node colors as set by the administrator.
  $colors = array();
  $color_defaults = variable_get("tripal_phylogeny_org_colors", array('1' => array('organism' => '', 'color' => '')));
  foreach ($color_defaults as $i => $details) {
    if ($details['organism']) {
      $colors[$details['organism']] =  $details['color'];
    }
  }
  $colors = json_encode($colors);

  // Add javascript data needed for this tree.
  drupal_add_js(
    ' // var having URL of json data source for charting
      var phylotreeDataURL =  baseurl + "/ajax/chado_phylotree/' . $phylotree->phylotree_id . '/json";

      // var with path to our theme, for use by javascript functions.
      var pathToTheme = baseurl + "/' . $module_path . '/theme";

      // var with custom options
      var treeOptions = ' . $options . ';

      // var with the organism colors
      var organismColors = ' . $colors . ';',
    'inline'
  );

  if (!property_exists($phylotree, 'has_nodes')) {
    // If the nodes haven't loaded then set a value so the template can
    // choose not to show the phylogram.
    $values  = array('phylotree_id' => $phylotree->phylotree_id);
    $options = array('limit' => 1, 'offset' => 0, 'has_record' => 1);
    $phylotree->has_nodes = chado_select_record('phylonode', array('phylonode_id'), $values, $options);
  }
  if (!property_exists($phylotree, 'has_features')) {
    // If the nodes haven't loaded then set a value so the template can
    // choose not to show the circular dendrogram. The chado_select_record()
    // API call can't do this query so we have to do it manually.
    $sql = "
      SELECT count(*) as num_features
      FROM {phylonode}
      WHERE NOT feature_id IS NULL and phylotree_id = :phylotree_id
      LIMIT 1 OFFSET 0
    ";
    $phylotree->has_features = chado_query($sql, array(':phylotree_id' => $phylotree->phylotree_id))->fetchField();
  }

  $phylotree->prepared_to_view = TRUE;
}
/**
 * Implements hook_preprocess_hook()
 *
 * @param $variables
 */
function tripal_phylogeny_preprocess_tripal_phylogeny_phylogram(&$variables) {
  $phylotree = $variables['node']->phylotree;
  tripal_phylogeny_prepare_tree_viewer($phylotree);
}

/**
 * Implements hook_preprocess_hook()
 *
 * @param $variables
 */
function tripal_phylogeny_preprocess_tripal_phylogeny_taxonomic_tree(&$variables) {
  $phylotree = $variables['node']->phylotree;
  tripal_phylogeny_prepare_tree_viewer($phylotree);
}
