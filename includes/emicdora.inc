<?php

/**
 * @file
 * Emicdora modifications.
 */
function collationtools_get_edited_collation($document_id) {
  module_load_include('inc', 'collation', 'includes/calliope');
  global $base_url;
  $file_query = array(
    'emic_data.collation' => $document_id,
    'emic_data.version1' => $_GET['version1'],
    'emic_data.version2' => $_GET['version2'],
    'emic_data.diff_kind' => $_GET['diff_kind'],
  );
  $file = emicdora_calliope_get_collation_file($file_query);
  if ($file) {
    echo $file['emic_data']['contents'];
    drupal_exit();
  }
  $calliope_url = 'html/comparison';
  $path = "$base_url/$calliope_url/$document_id";
  $url = url($path, array('query' => array(
      '_dc' => $_GET['_dc'],
      'version1' => $_GET['version1'],
      'version2' => $_GET['version2'],
      'diff_kind' => $_GET['diff_kind'],
  )));
  $contents = file_get_contents($url);
  $spans = preg_split( '#</span>#', $contents, -1, PREG_SPLIT_DELIM_CAPTURE | PREG_SPLIT_NO_EMPTY);
  $fixed_content = collationtools_fix_calliope($spans, $_GET['diff_kind']);

  $file_params = array(
    'collation' => $document_id,
    'version1' => $_GET['version1'],
    'version2' => $_GET['version2'],
    'diff_kind' => $_GET['diff_kind'],
    'emicdora_count' => 0,
  );
  emicdora_calliope_write_collation($file_params, $fixed_content);
  echo $fixed_content;
}

/**
 * Repairs possibly damaged Calliope.
 *
 * @param array $spans
 *   array of spans from file
 * @param string $type
 *   'delete' or 'added'
 *
 * @return xml
 *   Repaired xml
 */
function collationtools_fix_calliope($spans, $type) {
  $br_token = '!emicdora_br_token!';
  $repaired = array();
  $repaired[] = array_shift($spans). "</span>";
  foreach ($spans as $span) {
    if (strpos($span, '<span') === 0) {
      $repaired[] = "$span</span>";
    }
    else {
      $repaired[] = "<span>$span</span>";
    }
  }
  // Add missing id's.
  $counter = 1;
  $xml = implode('', $repaired);
  $fixed_xml = str_replace($br_token, '<br />', $xml);
  $working_xml = "<div id = 'emicdora_wrapped'>$fixed_xml</div>";
  $dom = new DOMDocument();
  $dom->loadHTML($working_xml);

  $spans = $dom->getElementsByTagName('span');
  foreach ($spans as $span) {
    $id = $span->getAttribute('id');
    if ($id == '') {
      $id = $type . $counter;
      $span->setAttribute('id', $id);
      $counter++;
    }
  }
  collationtools_defrag($dom, $spans);
  $wrapper = $dom->getElementById('emicdora_wrapped');
  $child_nodes = $wrapper->childNodes;
  $output = '';
  foreach ($child_nodes as $child) {
    $output .= $child->ownerDocument->saveXML($child);
  }
  return $output;
}

/**
 * Redefines granularity to whole words
 *
 * @param DOMDocument $dom
 *   Representation of entire document
 * @param DOMNodeList $spans
 *   List of span elements
 */
function collationtools_defrag($dom, $spans) {
  $mappings = array(
    'a' => 'added',
    'd' => 'deleted',
  );
  for ($item = 0; $item < $spans->length - 1; $item++) {
    $current = $spans->item($item);
    if ($current->getAttribute('class') == 'merged') {
      $current_contents = collationtools_get_innerHTML($dom, $current);
      $id = $current->getAttribute('id');
      $class = $mappings[$id[0]];
      $state = collationtools_find_space($current_contents);
      $space_count = substr_count($current->nodeValue, ' ');
      switch ($state) {
        case 'START':
          if ($item < $spans->length - 2) {
            $next_item = $spans->item($item + 1);
            $next_contents = collationtools_get_innerHTML($dom, $next_item);
            $next_state = collationtools_find_space($next_contents);
            if ($next_state != 'START' && $next_state != 'BOTH' && $space_count < 2) {
              $current->setAttribute('class', $class);
            }
          }
          break;

        case 'END':
          if ($item > 0) {
            $previous_element = $spans->item($item - 1);
            $previous_content = collationtools_get_innerHTML($dom, $previous_element);
            $previous_state = collationtools_find_space($previous_content);
            if ($previous_state != 'END' && $previous_state != 'BOTH' && $space_count < 2) {
              $current->setAttribute('class', $class);
            }
          }
          break;

        case FALSE:
          $altered = FALSE;
          $next_state = FALSE;
          $previous_state = FALSE;
          if ($item < $spans->length - 2) {
            $next_item = $spans->item($item + 1);
            $next_contents = collationtools_get_innerHTML($dom, $next_item);
            $next_state = collationtools_find_space($next_contents);
          }
          if ($item > 0) {
            $previous_element = $spans->item($item - 1);
            $previous_content = collationtools_get_innerHTML($dom, $previous_element);
            $previous_state = collationtools_find_space($previous_content);
          }

          if ($next_state != 'START' && $next_state != 'BOTH' && $space_count < 2) {
            $altered = TRUE;
          }
          if ($previous_state != 'END' && $previous_state != 'BOTH' && $space_count < 2) {
            $altered = TRUE;
          }
          if ($altered) {
            $current->setAttribute('class', $class);
          }
          break;
      }
    }
  }
}

/**
 * Locates space or br tag in input string.
 *
 * @param string $string
 *   String being searched
 *
 * @return string
 *   Space or br position
 */
function collationtools_find_space($string) {
  $br = '<br>';
  $starts_with_br = strpos($string, $br) === 0 ? TRUE : FALSE;
  $ends_with_br = substr($string, -strlen($br)) === $br ? TRUE : FALSE;
  $space_at_start = FAlSE;
  $space_at_end = FALSE;
  $state = FALSE;
  if (strlen($string) != strlen(ltrim($string)) || $starts_with_br) {
    $state = 'START';
    $space_at_start = TRUE;
  }
  if (strlen($string) != strlen(rtrim($string)) || $ends_with_br) {
    $state = 'END';
    $space_at_end = TRUE;
  }
  if ($space_at_start && $space_at_end) {
    $state = 'BOTH';
  }
  return $state;
}

/**
 * Gets innerHTML of DOMElement
 *
 * @param DOMDocument $dom
 *   Enclosing DOM
 * @param DOMElement $element
 *   Element being examined
 *
 * @return string
 *   InnerHTML
 */
function collationtools_get_innerHTML($dom, $element) {
  $html = '';
  foreach ($element->childNodes as $child) {
    $html .= $dom->saveHTML($child);
  }
  return $html;
}
