<?php
/**
 * @file
 * Displays a Vimeo pane.
 */

$plugin = array(
  'title' => t('Vimeo Pane'),
  'description' => t('Show random 3 Vimeo videos from the local site'),
  'category' => array('Vimeo', -10),
  'single' => TRUE,
  'key' => 'vimeo_pane',
  'edit form' => 'vimeo_pane_content_type_edit_form',
  'render callback' => 'vimeo_pane_content_type_render',
  'all contexts' => TRUE,
);

/* --- HOOKS ---------------------------------------------------------------- */

/**
 * Output function for the vimeo pane content type. Outputs a custom
 * based on the module and delta supplied in the configuration.
 */
function vimeo_pane_content_type_render($subtype, $conf, $args, $contexts) {
  $block          = new stdClass();
  $block->content = '';

  // Perfomance debug info
  // $start = microtime(TRUE);

  // Get random 3 nodes based on selected field.
  $results = db_select('field_revision_field_vimeo_field', 'fvf')
    ->fields('fvf', array('entity_id'))
    ->range(0,3)
    ->orderRandom()
    ->execute()
    ->fetchAll();

  // Create nids array for render.
  if ($results) {
    $nodes = array();
    foreach ($results as $result) {
      $nids[] = $result->entity_id;
    }
  }

  $block->content = render(node_view_multiple(node_load_multiple($nids), $conf['display_mode']));

  // Perfomance debug info
  // $block->content .= '<br>' . t('Executed in @time sec', array('@time' => round(microtime(TRUE) - $start, 2)));
  return $block;
}

/**
 * Returns an edit form for the custom content type.
 */
function vimeo_pane_content_type_edit_form($form, &$form_state) {
  $conf = $form_state['conf'];

  // Define bundles that will be used.
  foreach (node_type_get_types() as $key => $value) {
    $options[$key] = $value->name;
  }

  $form['settings'] = array(
    '#type'  => 'fieldset',
    '#title' => 'Vimeo pane settings'
  );
  // Let's create select list with existing content types.
  $form['settings']['content'] = array(
    '#type'          => 'select',
    '#title'         => t('Available content types') ,
    '#default_value' =>  $conf['content'],
    '#options'       => $options,
    '#requeried'     => TRUE,
  );

  // Get fields for each CT.
  foreach ($options as $entity_type => $label) {

    // Create option list for each content type.
    $field_list = vimeo_get_entity_fields('node', $entity_type, 'readable');

    // Create fields select list based on selected CT.
    $form['settings'][$entity_type . '_field_list'] = array(
      '#type'              => 'select',
      '#title'             => t('List of available fields'),
      '#description'       => t('You need to select field that will be used for render.'),
      '#options'           => $field_list,
      '#default_value'     => $conf[$entity_type . '_field_list'],
      '#requeried'         => TRUE,
      '#states'            => array(
        'visible'          => array(
          '#edit-content'  => array('value' => $entity_type),
        ),
      ),
    );
  }

  // Let's create select list with existing content types.
  $form['settings']['display_mode'] = array(
    '#type'          => 'select',
    '#title'         => t('Display mode') ,
    '#default_value' =>  $conf['display_mode'],
    '#options'       => array(
      'teaser'       => t('Teaser'),
      'default'      => t('Default'),
    ),
    '#requeried'     => TRUE,
  );

  return $form;
}

/**
 * The submit form stores the data in $conf.
 */
function vimeo_pane_content_type_edit_form_submit(&$form, &$form_state) {
  // Save config values.
  if (isset($form_state['values'])) {
    $form_state['conf'] = $form_state['values'];
  }
}
