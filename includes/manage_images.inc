<?php

/**
 * @file
 * Handles the upload of a single page.
 */

/**
 * Sequence of forms for handling reordering, deletion, or other batch
 * processes of images within collection.
 */
function chillco_islandora_solution_pack_multi_image_manage_images_menu(AbstractObject $object) {
  module_load_include('inc', 'islandora', 'includes/breadcrumb');
  module_load_include('inc', 'islandora_paged_content', 'includes/manage_pages');
  drupal_set_breadcrumb(islandora_get_breadcrumbs($object));
  drupal_set_title($object->label);
  return array(
    'manage_pages' => array(
      '#type' => 'vertical_tabs',
      'sequence' => array(
        '#access' => user_access(FEDORA_METADATA_EDIT),
        '#title' => t('Reorder Pages'),
        '#type' => 'fieldset',
        'form' => drupal_get_form('islandora_paged_content_manage_pages_sequences_form', $object),
        '#collapsible' => TRUE,
        '#collapsed' => TRUE,
      ),
      'delete' => array(
        '#access' => user_access(FEDORA_PURGE),
        '#title' => t('Delete Pages'),
        '#type' => 'fieldset',
        'form' => drupal_get_form('islandora_paged_content_manage_pages_delete_form', $object),
        '#collapsible' => TRUE,
        '#collapsed' => TRUE,
      ),
    ),
  );
}

/**
 * Local menu action to present an ingest page form.
 *
 * @param AbstractObject $object
 *   The record container to ingest into.
 *
 * @return string
 *   The HTML representation of the ingest page form.
 */
function chillco_islandora_solution_pack_multi_image_ingest_item(AbstractObject $object) {
  global $user;
  module_load_include('inc', 'islandora', 'includes/breadcrumb');
  drupal_set_breadcrumb(islandora_get_breadcrumbs($object));
  module_load_include('inc', 'islandora', 'includes/utilities');
  module_load_include('inc', 'islandora', 'includes/ingest.form');
  $tuque = islandora_get_tuque_connection();
  $image_item = $tuque->repository->constructObject(islandora_get_namespace($object->id));
  $image_item->owner = $user->name;
  $image_item->label = 'New Image';
  $image_item->models = 'islandora:chillco_isp_multi_image_item_cmodel';
  drupal_set_title(t('Add image to @container', array('@container' => $object->label)));
  return drupal_get_form('islandora_ingest_form', array(
    'multi_image_container' => $object,
    'models' => array('islandora:chillco_isp_multi_image_item_cmodel'),
    'object' => $image_item,
    'parent' => $object->id,
  ));
}

/**
 * The form for uploading a single page image.
 *
 * @param array $form
 *   The Drupal form definition.
 * @param array $form_state
 *   The Drupal form state.
 */
function chillco_islandora_solution_pack_multi_image_upload_item_form(array $form, array &$form_state, AbstractObject $parent) {
  $get_default_value = function($name, $default) use(&$form_state) {
    return isset($form_state['values'][$name]) ? $form_state['values'][$name] : $default;
  };
  $form_state['parent'] = $parent;
  $upload_size = min((int) ini_get('post_max_size'), (int) ini_get('upload_max_filesize'));
  $extensions = array('tiff tif jpg jpeg png gif');
  return array(
    'file' => array(
      '#title' => t('Image'),
      '#type' => 'managed_file',
      '#description' => t('Select an image to upload.<br/>Files must be less than <b>@size MB.</b><br/>Allowed file types: <b>@ext.</b>', array('@size' => $upload_size, '@ext' => $extensions[0])),
      '#default_value' => $get_default_value('files', NULL),
      '#upload_location' => 'temporary://',
      '#required' => TRUE,
      '#upload_validators' => array(
        'file_validate_extensions' => $extensions,
        'file_validate_size' => array($upload_size * 1024 * 1024),
      ),
    ),
  );
}

/**
 * Upload multiple images to record container.
 */
function chillco_islandora_solution_pack_multi_image_ingest_multiple_items_form(array $form, array &$form_state, AbstractObject $record_container) {
  module_load_include('inc', 'islandora', 'includes/breadcrumb');
  module_load_include('inc', 'islandora', 'includes/utilities');
  module_load_include('inc', 'islandora_paged_content', 'includes/utilities');
  drupal_set_breadcrumb(islandora_get_breadcrumbs($record_container));
  $current_images = islandora_paged_content_get_pages($record_container);
  $last_image_sequence_number = count($current_images);
  $form_state['container'] = $record_container;
  $message = t("This sequence currently has @count images. Additional images will be appended to the end of the sequence by default. You may reorder your images after the process is complete.", array("@count" => $last_image_sequence_number));

  $form['insertion_point'] = array(
    '#type' => 'value',
    '#value' => $last_image_sequence_number,
  );

  // Value behaves more consistently when passed as a string.
  $form['current_images'] = array(
    '#type' => 'hidden',
    '#value' => serialize($current_images),
  );

  $extensions = array('tiff tif jpg jpeg png gif');
  $form['files'] = array(
    '#title' => t('Compressed images file.'),
    '#type' => 'plupload',
    '#required' => TRUE,
    '#description' => t('Select file(s) for upload.<br/>'),
    '#default_value' => isset($form_state['values']['files']) ? $form_state['values']['files'] : NULL,
    '#upload_validators' => array(
      'file_validate_extensions' => $extensions,
    ),
  );

  $form['record_container_pid'] = array(
    '#type' => 'hidden',
    '#value' => $record_container->id,
  );
  $form['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Add images'),
  );
  return $form;

}

function chillco_islandora_solution_pack_multi_image_ingest_multiple_items_form_submit(array $form, array &$form_state) {
  dpm($form_state);
}

/**
 * Sets RELS-EXT/RELS-INT properties and creates the 'OBJ' datastream.
 *
 * @param array $form
 *   The Drupal form definition.
 * @param array $form_state
 *   The Drupal form state.
 */
function chillco_islandora_solution_pack_multi_image_upload_item_form_submit(array $form, array &$form_state) {
  module_load_include('inc', 'islandora_paged_content', 'includes/utilities');
  $object = islandora_ingest_form_get_object($form_state);
  $file = file_load($form_state['values']['file']);
  $object->label = $file->filename;
  $file_info = pathinfo($object->label);
  $dsid = 'IMAGE';
  if (strtolower($file_info['extension']) == 'tif' || strtolower($file_info['extension']) == 'tiff') {
    $dsid = 'OBJ';
  }
  islandora_paged_content_update_datastream($object, drupal_realpath($file->uri), $dsid, NULL, NULL, 'M', FALSE);
  // Update RELS-EXT properties, page/sequence/etc, and append the page at the
  // end of the record container.
  $rels_ext = $object->relationships;
  $parent = $form_state['parent'];
  $pages = islandora_paged_content_get_pages($parent);
  $num_pages = count($pages) + 1;
  islandora_paged_content_set_relationship($rels_ext, ISLANDORA_RELS_EXT_URI, 'isPageOf', $parent->id);
  islandora_paged_content_set_relationship($rels_ext, ISLANDORA_RELS_EXT_URI, 'isSequenceNumber', (string) $num_pages, TRUE);
  islandora_paged_content_set_relationship($rels_ext, ISLANDORA_RELS_EXT_URI, 'isPageNumber', (string) $num_pages, TRUE);
  islandora_paged_content_set_relationship($rels_ext, ISLANDORA_RELS_EXT_URI, 'isSection', '1', TRUE);
  islandora_paged_content_set_relationship($rels_ext, FEDORA_RELS_EXT_URI, 'isMemberOf', $parent->id);
}
