<?php

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
 *   The book to ingest into.
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
  $image_item->models = 'islandora:pageCModel';
  drupal_set_title(t('Add image to @container', array('@container' => $object->label)));
  return drupal_get_form('islandora_ingest_form', array(
    'book' => $object,
    'models' => array($image_item->models),
    'object' => $image_item,
    'parent' => $object->id,
  ));
}
