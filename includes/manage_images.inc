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

  drupal_set_message(t("This sequence currently has @count images. Additional images will be appended to the end of the sequence by default sorted by filename. You may reorder your images after the process is complete.", array("@count" => $last_image_sequence_number)));
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
  $tuque = new IslandoraTuque();
  $repository = $tuque->repository;
  $values = $form_state['values'];
  $record_container_pid = $values['record_container_pid'];
  $namespace = substr($record_container_pid, 0, strpos($record_container_pid, ":"));
  $record_container = islandora_object_load($record_container_pid);
  $insertion_point = $values['insertion_point'];
  if (!$record_container) {
    drupal_set_message(t("This container does not exist in this repository"), 'warning');
    return;
  }

  $files = array();
  foreach ($values['files'] as $temporary_file) {
    if ($temporary_file['status'] == 'done') {
      $files[$temporary_file['name']] = $temporary_file;
    }
  }
  ksort($files);

  $status_message = t('Adding @count image(s) to container.', array('@count' => count($files)));
  $batch = array(
    'title' => $status_message,
    'progress_message' => t('Completed @current operations out of @total.'),
    'operations' => array(),
    'file' => drupal_get_path('module', 'chillco_islandora_solution_pack_multi_image') . '/includes/manage_images.inc',
    'finished' => 'chillco_islandora_solution_pack_multi_image_ingest_multiple_items_finished',
  );

  $config = array(
    'record_container_pid' => $record_container_pid,
    'namespace' => $namespace,
  );
  foreach ($files as $file) {
    $config['sequence_number'] = ++$insertion_point;
    $config['image'] = $file;
    $batch['operations'][] = array(
      'chillco_islandora_solution_pack_multi_image_add_image_items',
      array($repository, $config),
    );
  }

  batch_set($batch);
  batch_process();
}

/**
 * Sets RELS-EXT/RELS-INT properties and creates the 'OBJ' datastream.
 */
function chillco_islandora_solution_pack_multi_image_add_image_items($repository, $config, &$context) {
  module_load_include('inc', 'islandora', 'includes/IslandoraTuque');
  module_load_include('inc', 'islandora_paged_content', 'includes/utilities');

  $file_info = pathinfo($config['image']['name']);
  // Create object.
  $object = $repository->constructObject($config['namespace']);
  $object->label = $file_info['filename'];
  $dsid = 'IMAGE';
  if (strtolower($file_info['extension']) == 'tif' || strtolower($file_info['extension']) == 'tiff') {
    $dsid = 'OBJ';
  }
  islandora_paged_content_update_datastream($object, drupal_realpath($config['image']['tmppath']), $dsid, NULL, NULL, 'M', FALSE);
  $rels_ext = $object->relationships;
  $parent = $config['record_container_pid'];
  $object->relationships->add(FEDORA_MODEL_URI, 'hasModel', 'islandora:chillco_isp_multi_image_item_cmodel');
  islandora_paged_content_set_relationship($rels_ext, ISLANDORA_RELS_EXT_URI, 'isPageOf', $parent);
  islandora_paged_content_set_relationship($rels_ext, ISLANDORA_RELS_EXT_URI, 'isSequenceNumber', (string) $config['sequence_number'], TRUE);
  islandora_paged_content_set_relationship($rels_ext, ISLANDORA_RELS_EXT_URI, 'isPageNumber', (string) $config['sequence_number'], TRUE);
  islandora_paged_content_set_relationship($rels_ext, ISLANDORA_RELS_EXT_URI, 'isSection', '1', TRUE);
  islandora_paged_content_set_relationship($rels_ext, FEDORA_RELS_EXT_URI, 'isMemberOf', $parent);
  islandora_add_object($object);
  $context['results']['tmp_files'][] = $config['image']['tmppath'];
}

/**
 * Batch 'finished' callback.
 *
 * Deletes the temporary files associated with the ingested images.
 */
function chillco_islandora_solution_pack_multi_image_ingest_multiple_items_finished($success, $results, $operations) {
  if (!isset($rsults['tmp_files'])) {
    return;
  }
  foreach ($results['tmp_files'] as $file_path) {
    @unlink(drupal_realpath($file_path));
  }
}
