<?php
/**
 * Provide some handlers for vimeo field
 */

/**
 * Helper function to get the Vimeo video info
 *
 * @param string $url
 *   A Vimeo video URL
 *
 * @return array
 *   The video info, or FALSE if unable to get the video
 */
function vimeo_get_video_info($url) {
  $start = microtime(TRUE);

  // Gte response from Vimeo.
  $response = drupal_http_request('http://vimeo.com/api/oembed.json?url=' . $url);

  // Log execution performance data.
  watchdog('Vimeo', '@FINISHED: Response received in <strong>@time</strong> sec.', array('@time' => round(microtime(TRUE) - $start, 2)), WATCHDOG_WARNING);

  if ($response->code == '200' && !isset($response->error)) {
    return json_decode($response->data);
  }

  return FALSE;
}

/**
 * Helper function that returns list of fields by given entity.
 * @param string $entity_bundle
 *   The Entity bundle to return fields for.
 * @param string $entity_type
 *   The Content type to return for.
 * @param string $options
 *   The readable/machine_name options to choose for labels in return array.
 * @return array
 */
function vimeo_get_entity_fields($entity_bundle, $entity_type, $options) {
  // Use core functionality to build fileds array.
  $instances = field_info_instances($entity_bundle, $entity_type);
  $extra_fields = field_info_extra_fields($entity_bundle, $entity_type, 'form');
  if ($extra_fields) {
    $instances = array_merge($extra_fields, $instances);
  }

  // Let's create array of fields for given entity.
  foreach ($instances as $field_machine_name => $instance) {
    if ($options) {
      switch ($options) {
        case 'readable':
          $fields[$field_machine_name] = $instance['label'];
          break;
        case 'machine_name':
          $fields[$field_machine_name] = $field_machine_name;
          break;
      }
    }
  }

  return $fields;
}
