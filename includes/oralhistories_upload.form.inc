<?php

/**
 * @file
 * Handles the uploading of the video file.
 */

/**
 * Defines a file upload form for uploading the islandora video/audio files.
 *
 * @param array $form
 *   The drupal form.
 * @param array $form_state
 *   The drupal form state.
 *
 * @return array
 *   The drupal form definition.
 */
function islandora_oralhistories_upload_form(array $form, array &$form_state) {
  $upload_size = min((int) ini_get('post_max_size'), (int) ini_get('upload_max_filesize'));
  $media_extensions = array('ogg mp4 mov qt m4v avi mkv wav mp3');
  $transcript_extensions = array('vtt xml');
  return array(
    'media_file' => array(
      '#title' => t('Video or Audio'),
      '#type' => 'managed_file',
      '#required' => TRUE,
      '#description' => t('Select video or audio file to upload.<br/>Files must be less than <b>@size MB.</b><br/>Allowed file types: <b>@ext.</b>', array('@size' => $upload_size, '@ext' => $media_extensions[0])),
      '#default_value' => isset($form_state['values']['file']) ? $form_state['values']['file'] : NULL,
      '#upload_location' => 'temporary://',
      '#upload_validators' => array(
        'file_validate_extensions' => $media_extensions,
         // Assume its specified in MB.
        'file_validate_size' => array($upload_size * 1024 * 1024),
      ),
    ),

    'transcript_file' => array(
      '#title' => t('Transcript file'),
      '#type' => 'managed_file',
      '#required' => TRUE,
      '#description' => t('Select transcript file to upload.<br/>File name must end with two-letter language code (valid BCP 47 language tag), like "en" for English.<br/>Files must be less than <b>@size MB.</b><br/>Allowed file types: <b>@ext.</b>', array('@size' => $upload_size, '@ext' => $transcript_extensions[0])),
      '#default_value' => isset($form_state['values']['transcript_file']) ? $form_state['values']['transcript_file'] : NULL,
      '#upload_location' => 'temporary://',
      '#upload_validators' => array(
        'file_validate_extensions' => $transcript_extensions,
        // Assume its specified in MB.
        'file_validate_size' => array($upload_size * 1024 * 1024),
      ),
    ),
/*
    'transcript_xslt_file' => array(
      '#title' => t('Transcript XSLT file'),
      '#type' => 'managed_file',
      '#required' => FALSE,
      '#description' => t('Select transcript XSLT file to upload if your transcript xml file need custome XSLT to transform.<br/>Files must be less than <b>@size MB.</b><br/>Allowed file types: <b>@ext.</b>', array('@size' => $upload_size, '@ext' => $transcript_extensions[0])),
      '#default_value' => isset($form_state['values']['transcript_xslt_file']) ? $form_state['values']['transcript_xslt_file'] : NULL,
      '#upload_location' => 'temporary://',
      '#upload_validators' => array(
        'file_validate_extensions' => array('xslt'),
        // Assume its specified in MB.
        'file_validate_size' => array($upload_size * 1024 * 1024),
      ),
    ),
*/

  );
}



/**
 * Submit handler, adds uploaded file to the Oral Histories object.
 *
 * @param array $form
 *   The drupal form.
 * @param array $form_state
 *   The drupal form state.
 */
function islandora_oralhistories_upload_form_submit(array $form, array &$form_state) {
  $object = $form_state['islandora']['objects'][0];

  // Create OJB datastream
  if (empty($object['OBJ'])) {
    $ds = $object->constructDatastream('OBJ', 'M');
    $object->ingestDatastream($ds);
  }
  else {
    $ds = $object['OBJ'];
  }
  $media_file = file_load($form_state['values']['media_file']);
  $media_file_path = drupal_realpath($media_file->uri);
  $ds->setContentFromFile($media_file_path, FALSE);
  $ds->label = $media_file->filename;
  $ds->mimetype = $media_file->filemime;

  // Create TRANSCRIPT datastream
  if (empty($object['TRANSCRIPT'])) {
    $ds = $object->constructDatastream('TRANSCRIPT', 'M');
    $object->ingestDatastream($ds);
  }
  else {
    $ds = $object['TRANSCRIPT'];
  }
  $transcript_file = file_load($form_state['values']['transcript_file']);
  $transcript_file_path = drupal_realpath($transcript_file->uri);
  $ds->setContentFromFile($transcript_file_path, FALSE);
  $ds->label = $transcript_file->filename;
  // Drupal returns $transcript_file->filemime = 'application/octet-stream' incorrectly for .vtt file
  $transcript_ext = pathinfo($transcript_file->filename, PATHINFO_EXTENSION);
  if (strtolower($transcript_ext) == 'vtt'){
    $ds->mimetype = 'text/vtt';
  } else {
    $ds->mimetype = $transcript_file->filemime;
  }


  // record uploaded media file type
  if (preg_match('/video/', $media_file->filemime)) {
    $media_upload_type = 'video';
  } else {
    $media_upload_type = 'audio';
  }
  variable_set('islandora_oralhistories_media_uploaded_type', $media_upload_type);
}
