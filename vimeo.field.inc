<?php
/**
 * @file
 * Implements a vimeo field.
 *
 * @author Alexei Goja
 */

// Load our default handlers
module_load_include('inc', 'vimeo', 'vimeo.handlers');

/**
 * Implements hook_field_info().
 */
function vimeo_field_info() {
  return array(
    'vimeo_field' => array(
      'label' => 'Vimeo field',
      'description' => 'Videos from vimeo',
      'settings' => array(),
      'instance_settings' => array(
        'description_field' => 0,
        'description_length' => 128,
      ),
      'default_widget' => 'vimeo_field_video',
      'default_formatter' => 'vimeo_field',
      'property_type' => 'vimeo_field',
      'property_callbacks' => array('vimeo_property_info_callback'),
    ),
  );
}

/**
 * Property callback for the Entity Metadata framework.
 */
function vimeo_property_info_callback(&$info, $entity_type, $field, $instance, $field_type) {
  // Apply the default.
  entity_metadata_field_default_property_callback($info, $entity_type, $field, $instance, $field_type);

  // Finally add in instance specific property info.
  $name = $field['field_name'];
  $property = &$info[$entity_type]['bundles'][$instance['bundle']]['properties'][$name];
  $property['type'] = ($field['cardinality'] != 1) ? 'list<vimeo_field>' : 'vimeo_field';
  $property['property info'] = vimeo_field_data_property_info('Vimeo');
  $property['getter callback'] = 'entity_metadata_field_verbatim_get';
  $property['setter callback'] = 'entity_metadata_field_verbatim_set';
}

/**
 * Defines info for the properties of the video_embed_field data structure.
 */
function vimeo_field_data_property_info($name = NULL) {
  // Build an array of basic property information for vimeo field.
  $properties = array(
    'video_url' => array(
      'label' => 'Video URL',
      'type' => 'uri',
      'setter callback' => 'entity_property_verbatim_set',
    ),
    'thumbnail_path' => array(
      'label' => 'Thumbnail path',
      'type' => 'uri',
      'getter callback' => 'entity_property_verbatim_get_url'
    ),
    'video_id' => array(
      'label' => 'Description',
      'type' => 'text',
      'setter callback' => 'entity_property_verbatim_set',
    ),
  );

  // Add the default values for each of the vimeo field properties.
  foreach ($properties as $key => &$value) {
    $value += array(
      'description' => !empty($name) ? t('!label of field %name', array('!label' => $value['label'], '%name' => $name)) : '',
    );
  }

  return $properties;
}

/**
 * Gets the property just as it is set in the data and converts to absolute url.
 */
function entity_property_verbatim_get_url($data, array $options, $name, $type, $info) {
  $property = entity_property_verbatim_get($data, $options, $name, $type, $info);
  return file_create_url($property);
}

/**
 * Implements hook_field_widget_info().
 */
function vimeo_field_widget_info() {
  return array(
    'vimeo_field_video' => array(
      'label' => 'Vimeo widget',
      'description' => 'Provides a vimeo field',
      'field types' => array('vimeo_field'),
      'settings' => array(),
      'behaviors' => array(
        'multiple values' => FIELD_BEHAVIOR_DEFAULT,
        'default value' => FIELD_BEHAVIOR_DEFAULT,
      ),
    ),
  );
}

/**
 * Implements hook_field_widget_form().
 */
function vimeo_field_widget_form(&$form, &$form_state, $field, $instance, $langcode, $items, $delta, $element) {
  // Don't need to check the type right now because we're only defining one
  $element += array(
    '#type' => 'fieldset',
  );

  $element['video_url'] = array(
    '#type' => 'textfield',
    '#title' => t('Video URL'),
    '#default_value' => isset($items[$delta]['video_url']) ? $items[$delta]['video_url'] : '',
    '#required' => $element['#required'],
  );

  return $element;
}

/**
 * Validates video URL.
 */
function vimeo_field_validate($entity_type, $entity, $field, $instance, $langcode, $items, &$errors) {
  foreach ($items as $delta => $item) {
    if (!empty($item['video_url'])) {
      $item['video_url'] = trim($item['video_url']);
      if (stristr($item['video_url'], '.') && !stristr($item['video_url'], 'http://') && !stristr($item['video_url'], 'https://')) {
        $item['video_url'] = 'http://' . $item['video_url'];
      }

      $parts = parse_url($item['video_url']);

      if (!$parts || !isset($parts['host'])) {
        $errors[$field['field_name']][$langcode][$delta][] = array(
          'error' => t('Invalid Url'),
          'message' => t('Video: Invalid Video URL.', array('%name' => $instance['label'])),
        );
      }
      else {
        $host = $parts['host'];
        if (stripos($host, 'www.') > -1) {
          $host = substr($host, 4);
        }

        if ($host != 'vimeo.com') {
          $errors[$field['field_name']][$langcode][$delta][] = array(
            'error' => t('Unsupported Video Provider'),
            'message' => t('%name: This video provider is not currently supported.', array('%name' => $instance['label'])),
          );
        }
      }
    }
  }
}

/**
 * Implementation of hook_field_presave().
 *
 * Download and save the thumbnail if it hasn't already been stored.
 * Get video data.
 */
function vimeo_field_presave($entity_type, $entity, $field, $instance, $langcode, &$items) {
  foreach ($items as $delta => $item) {
    // Trim whitespace from the video URL.
    $items[$delta]['video_url'] = trim($item['video_url']);

    // Try to load thumbnail URL
    $info = vimeo_get_video_info($item['video_url']);

    if (isset($info->thumbnail_url) && $info->thumbnail_url) {
      $thumb_url = $info->thumbnail_url;
      $local_path = 'public://vimeo/' . $info->video_id . '.jpg';

      $dirname = drupal_dirname($local_path);
      file_prepare_directory($dirname, FILE_CREATE_DIRECTORY | FILE_MODIFY_PERMISSIONS);

      $response = drupal_http_request($thumb_url);

      if (!isset($response->error)) {
        file_save_data($response->data, $local_path, TRUE);
      }
      else {
        @copy($thumb_url, $local_path);
      }

      $items[$delta]['thumbnail_path'] = $local_path;
    }
    // Couldn't get the thumbnail for whatever reason
    else {
      $items[$delta]['thumbnail_path'] = '';
    }

    // @todo: Save file locally.
    if (isset($info)) {
      // $items[$delta]['thumbnail_path'] = $info->thumbnail_url;
      $items[$delta]['video_id'] = $info->video_id;

    }
  }
}

/**
 * Implements hook_field_is_empty().
 */
function vimeo_field_is_empty($item, $field) {
  return empty($item) || empty($item['video_url']) || $item['video_url'] === '';
}

/**
 * Implements hook_field_formatter_info().
 */
function vimeo_field_formatter_info() {
  $info = array(
    'vimeo_field_thumbnail' => array(
      'label' => t('Thumbnail Preview'),
      'field types' => array('vimeo_field'),
      'settings' => array(
        'image_style' => '',
        'description' => 1,
        'description_position' => 'bottom',
        'image_link' => 'none',
      ),
    ),
  );
  return $info;
}

/**
 * Implements hook_field_formatter_settings_form().
 */
function vimeo_field_formatter_settings_form($field, $instance, $view_mode, $form, &$form_state) {
  $display = $instance['display'][$view_mode];
  $settings = $display['settings'];
  $element = array();

  if ($display['type'] == 'vimeo_field_thumbnail') {
    $element['image_style'] = array(
      '#title' => t('Image style'),
      '#type' => 'select',
      '#options' => image_style_options(FALSE),
      '#default_value' => $settings['image_style'],
      '#empty_option' => t('None (original image)'),
    );
  }

  if ($display['type'] == 'vimeo_field_thumbnail') {

    $link_types = array(
      'content' => t('Content'),
      'source' => t('Video Source'),
    );

    $element['image_link'] = array(
      '#title' => t('Link thumbnail to'),
      '#type' => 'select',
      '#default_value' => $settings['image_link'],
      '#empty_option' => t('Nothing'),
      '#options' => $link_types,
    );
  }

  return $element;
}

/**
 * Implements hook_field_formatter_settings_summary().
 */
function vimeo_field_formatter_settings_summary($field, $instance, $view_mode) {
  $display = $instance['display'][$view_mode];
  $settings = $display['settings'];

  $summary = array();

  if ($display['type'] == 'vimeo_field_thumbnail') {
    $image_styles = image_style_options(FALSE);
    // Image style summary.
    if (isset($image_styles[$settings['image_style']])) {
      $summary[] = t('Image style: @style', array('@style' => $image_styles[$settings['image_style']]));
    }
    // No Image style (original image)
    else {
      $summary[] = t('Original Image.');
    }

    // Link summary.
    if (isset($settings['image_link']) && !empty($settings['image_link'])) {
      $summary[] = t('Image link: ' . $settings['image_link']);
    }
    else {
      $summary[] = t('Image link: NONE');
    }
  }

  return implode('<br />', $summary);
}

/**
 * Implements hook_field_formatter_view().
 */
function vimeo_field_formatter_view($entity_type, $entity, $field, $instance, $langcode, $items, $display) {
  $element = array();
  $settings = $display['settings'];

  if ($display['type'] == 'vimeo_field_thumbnail' && $display['settings']['image_link'] == 'content') {
    $path = entity_uri($entity_type, $entity);
  }

  foreach ($items as $delta => $item) {
    // Render the field.
    if ($display['type'] == 'vimeo_field_thumbnail') {
      if (isset($item['thumbnail_path'])) {
        if ($display['settings']['image_link'] == 'source') {
          $path = array(
            'path' => $item['video_url'],
          );
        }

        $element[$delta] = array(
          '#theme' => 'image_formatter',
          '#item' => array('uri' => $item['thumbnail_path']),
          '#image_style' => $display['settings']['image_style'],
          '#path' => isset($path) ? $path : '',
        );
      }
      else {
        $element[$delta] = array();
      }
    }
  }
  return $element;
}