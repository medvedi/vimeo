<?php
/**
 * Provide some handlers for vimeo embed field
 */

/**
 * Helper function to get the Vimeo video's ID
 *
 * @param string $url
 *   A Vimeo video URL to get the ID of
 *
 * @return array
 *   The video info, or FALSE if unable to get the video
 */
function vimeo_get_video_info($url) {
  $start = microtime(TRUE);

  // Gte response from VIMEO.
  $response = drupal_http_request('http://vimeo.com/api/oembed.json?url=' . $url);

  // Log execution performance data.
  watchdog('Vimeo', '@FINISHED: Response received in <strong>@time</strong> sec.', array('@time' => round(microtime(TRUE) - $start, 2)), WATCHDOG_WARNING);

  if ($response->code == '200' && !isset($response->error)) {
    return json_decode($response->data);
  }

  return FALSE;
}
